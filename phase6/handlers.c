#include "spede.h"
#include "types.h"
#include "handlers.h"
#include "tools.h"
#include "data.h"
#include "proc.h"


void NewProcHandler(func_ptr_t p) {
  int pid;

  if (free_q.size == 0) {
    cons_printf("Kernel Panie: no more PID left!\n");
    return;
  }

  pid = DeQ(&free_q);

  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  MyBzero((char *)&pcb[pid], PROC_STACK_SIZE);

  pcb[pid].state = READY;
  if (pid != 0){
    ch_p[pid*80+43] =0xf00 + 'r';
  }
  if (pid > 9){
    ch_p[pid*80+40] = 0xf00 + 1 + '0';
    ch_p[pid*80+41] = 0xf00 + (pid - 10) + '0';
  } else{
    ch_p[pid*80+41] = 0xf00 + pid + '0';
  }
  EnQ(pid, &ready_q);


  pcb[pid].TF_p = (TF_t *)&proc_stack[pid][PROC_STACK_SIZE];
  pcb[pid].TF_p--;
  pcb[pid].TF_p->eip = (unsigned int)p;
  pcb[pid].TF_p->eflags = EF_DEFAULT_VALUE | EF_INTR;
  pcb[pid].TF_p->cs = get_cs();
  pcb[pid].TF_p->ds = get_ds();
  pcb[pid].TF_p->es = get_es();
  pcb[pid].TF_p->fs = get_fs();
  pcb[pid].TF_p->gs = get_gs();
}


void TimerHandler(void) {
 int i;
  pcb[current_pid].cpu_time++;
 current_time++;

 for(i = 0; i < PROC_NUM ; i++){
  if(pcb[i].state == SLEEP && pcb[i].wake_time == current_time){
   pcb[i].state = READY;
      ch_p[i*80+43] =0xf00 + 'r';
   EnQ(i, &ready_q);
  }
 }

 if (pcb[current_pid].cpu_time == TIME_LIMIT) {
    pcb[current_pid].state = READY;
    ch_p[current_pid*80+43] =0xf00 + 'r';
    EnQ(current_pid, &ready_q);
    current_pid = 0;
  }

  outportb(0x20, 0x60);
}

void GetPidHandler(void) {
  pcb[current_pid].TF_p->eax = current_pid;
}

void SleepHandler(void) {
  pcb[current_pid].wake_time = current_time + (100*pcb[current_pid].TF_p->eax);
  pcb[current_pid].state = SLEEP;
    ch_p[current_pid*80+43] =0xf00 + 'S';
  current_pid = 0;
}

void SemAllocHandler(int passes){
    int i;
    for(i=0; i < Q_SIZE; i++){
      if (sem[i].owner==0){
        MyBzero((char *)&sem[i], sizeof(sem_t));
        sem[i].owner = current_pid;
        sem[i].passes=passes;
        ch_p[51] =0xf00 + passes + '0';
        pcb[current_pid].TF_p->ebx = i;
        break;
      }
    }
}

void SemWaitHandler(int sid){
    if (sem[sid].passes>0){
      sem[sid].passes--;
      ch_p[51] =0xf00 + sem[sid].passes + '0';
      return;
    }
    EnQ(current_pid, &(sem[sid].wait_q));
    pcb[current_pid].state = WAIT;
    ch_p[current_pid*80+43] =0xf00 + 'W';
    current_pid = 0;

}

void SemPostHandler(int sid){
    int process_number;
    if (sem[sid].wait_q.size > 0){
      process_number=DeQ(&(sem[sid].wait_q));
      pcb[process_number].state = READY;
      ch_p[process_number*80+43] =0xf00 + 'r';
      EnQ(process_number,&ready_q);
      return;
    }
    sem[sid].passes++;
    ch_p[51] =0xf00 + sem[sid].passes + '0';
}

void SysPrintHandler(char *p){
    int i, code;

    const int printer_port = 0x378;
    const int printer_data = printer_port + 0;
    const int printer_status = printer_port + 1;
    const int printer_control = printer_port + 2;

    outportb(printer_control, 16);
    code = inportb(printer_status);
    for(i=0; i<50; i++) asm("inb $0x80");
    outportb(printer_control, 4 | 8 );

    while(*p) {
        outportb(printer_data, *p);
        code = inportb(printer_control);
        outportb(printer_control, code | 1);
        for(i=0; i<50; i++) asm("inb $0x80");
        outportb(printer_control, code);

        for(i = 0; i < LOOP*3; i++) {
          code = inportb(printer_status) & 64;
          if(code == 0) break;
          asm("inb $0x80");
        }

        if(i == LOOP*3) {
          cons_printf(">>> Printer timed out!\n");
          break;
        }

        p++;
    }
}

void PortWriteOne(int port_num){
	char one;
	if (port[port_num].write_q.size == 0 && port[port_num].loopback_q.size == 0){
		port[port_num].write_ok = 1;
		return;
	}
	
	if (port[port_num].loopback_q.size > 0){
		one = DeQ(&(port[port_num].loopback_q));
	}else{
	  one = DeQ(&(port[port_num].write_q));
		SemPostHandler(port[port_num].write_sid);
	}
	outportb(port[port_num].IO+DATA, one);
	port[port_num].write_ok = 0;
}

void PortReadOne(int port_num){
	char one;
	one = inportb(port[port_num].IO + DATA);
	
	if (port[port_num].read_q.size == Q_SIZE){
		cons_printf("Kernel Panic: your typing on terminal is super fast!\n");
    return;
	}
	
	EnQ(one,&(port[port_num].read_q));
	EnQ(one,&(port[port_num].loopback_q));
	
	if (one == '\r'){
		EnQ('\n',&(port[port_num].loopback_q));
	}
	
	SemPostHandler(port[port_num].read_sid);	
}

void PortHandler(void){
	int port_num, intr_type;
  for (port_num = 0; port_num < PORT_NUM; port_num++){
    intr_type = inportb(port[port_num].IO + IIR);
    if (intr_type == IIR_RXRDY){
      PortReadOne(port_num);
    }
    if (intr_type == IIR_TXRDY){
      PortWriteOne(port_num);
    }
    if (port[port_num].write_ok == 1){
      PortWriteOne(port_num);
    }
  }
	
	outportb(0x20,0x63); //IRQ3
  outportb(0x20, 0x64);// IRQ4
}

void PortAllocHandler(int *eax){
	int port_num, baud_rate, divisor, i, port_found;
	static int IO[PORT_NUM] = { 0x2f8, 0x3e8, 0x2e8 };
	port_found = 0;
	for(i = 0; i < PORT_NUM; i++) {
      if (port[i].owner == 0){
			  port_num = i;
			  port_found = 1;
			  break;
      }
  }
	
	if (port_found == 0){
		cons_printf("Kernel Panic: no port left!\n");
    return;
	}
	//write port_num at where eax point to // service call can return it
  *eax = (unsigned int) port_num;
	//call MyBzero to clear the allocated port data
  MyBzero((char *)&port[port_num],sizeof(port_t));

	port[port_num].owner = current_pid;
	port[port_num].IO = IO[port_num];
	port[port_num].write_ok = 1;
	
	baud_rate = 9600;
  divisor = 115200 / baud_rate;
  outportb(port[port_num].IO+CFCR, CFCR_DLAB);
  outportb(port[port_num].IO+BAUDLO, LOBYTE(divisor));
  outportb(port[port_num].IO+BAUDHI, HIBYTE(divisor));
  outportb(port[port_num].IO+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
  outportb(port[port_num].IO+IER, 0);
  outportb(port[port_num].IO+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
  asm("inb $0x80");
  outportb(port[port_num].IO+IER, IER_ERXRDY|IER_ETXRDY);
} 

void PortWriteHandler(char one, int port_num) { // to buffer one, actually
  if (port[port_num].write_q.size == Q_SIZE){
        cons_printf("Kernel Panic: terminal is not prompting (fast enough)?\n");
        return;
	}
    
	EnQ(one, &(port[port_num].write_q));   
	if (port[port_num].write_ok == 1){
		PortWriteOne(port_num);
	}
}

void PortReadHandler(char *one, int port_num) { // to read from buffer, actually
  if (port[port_num].read_q.size == 0){
        cons_printf("Kernel Panic: nothing in typing/read buffer?\n");
        return;
	}
      *one = DeQ(&(port[port_num].read_q));
}

///////////////////////////////////////////////////// phase6 file services
void FSfindHandler(void) {
   char *name, *data;
   attr_t *attr_p;
   dir_t *dir_p;

   name = (char *)pcb[current_pid].TF_p->eax;
   data = (char *)pcb[current_pid].TF_p->ebx;

   dir_p = FSfindName(name);

   if(! dir_p) {      // dir_p == 0, not found
      data[0] = 0;    // null terminated, not found, return
      return;
   }

   attr_p = (attr_t *)data;
   FSdir2attr(dir_p, attr_p); // copy what dir_p points to to where attr_p points to

// should include filename (add 1 to length for null char)
   MyMemcpy((char *)(attr_p + 1), dir_p->name, MyStrlen(dir_p->name) + 1);
}

void FSopenHandler(void) {
   char *name;
   int fd;
   dir_t *dir_p;

   name = (char *)pcb[current_pid].TF_p->eax;

   fd = FSallocFD(current_pid);  // current_pid is owner of fd allocated

   if( fd == -1 ) {
      cons_printf("FSopenHandler: no more File Descriptor!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   dir_p = FSfindName(name);
   if(! dir_p) {
      cons_printf("FSopenHandler: name not found!\n");
      pcb[current_pid].TF_p->ebx = -1;
      return;
   }

   fd_array[fd].item = dir_p;        // dir_p is the name
   pcb[current_pid].TF_p->ebx = fd;  // process gets this to future read
}

// Copy bytes from file into user's buffer. Returns actual count of bytes
// transferred. Read from fd_array[fd].offset (initially given 0) for
// buff_size in bytes, and record the offset. may reach EOF though...
void FSreadHandler(void) {
   int fd, result, remaining;
   char *read_data;
   dir_t *lp_dir;

   fd = pcb[current_pid].TF_p->eax;
   read_data = (char *)pcb[current_pid].TF_p->ebx;

   if(! FScanAccessFD(fd, current_pid)) {
      cons_printf("FSreadHandler: cannot read from FD!\n");
      read_data[0] = 0;  // null-terminate it
      return;
   }

   lp_dir = fd_array[fd].item;

   if( A_ISDIR(lp_dir->mode ) ) {  // it's a dir
// if reading directory, return attr_t structure followed by obj name.
// a chunk returned per read. `offset' is index into root_dir[] table.
      dir_t *this_dir = lp_dir;
      attr_t *attr_p = (attr_t *)read_data;
      dir_t *dir_p;

      if( BUFF_SIZE < sizeof( *attr_p ) + 2) {
         cons_printf("FSreadHandler: read buffer size too small!\n");
         read_data[0] = 0;  // null-terminate it
         return;
      }

// use current dir, advance to next dir for next time when called
      do {
         dir_p = ((dir_t *)this_dir->data);
         dir_p += fd_array[fd].offset ;

         if( dir_p->inode == END_INODE ) {
            read_data[0] = 0;  // EOF, null-terminate it
            return;
         }
         fd_array[fd].offset++;   // advance
      } while(dir_p->name == 0);

// MyBzero() fills buff with 0's, necessary to clean buff
// since FSdir2attr may not completely overwrite whole buff...
      MyBzero(read_data, BUFF_SIZE);
      FSdir2attr(dir_p, attr_p);

// copy obj name after attr_t, add 1 to length for null
      MyMemcpy((char *)( attr_p + 1 ), dir_p->name, MyStrlen( dir_p->name ) + 1);

   } else {  // a file, not dir
// compute max # of bytes can transfer then MyMemcpy()
      remaining = lp_dir->size - fd_array[fd].offset;

      if( remaining == 0 ) {
         read_data[0] = 0;  // EOF, null-terminate it
         return;
      }

      MyBzero(read_data, BUFF_SIZE);  // null termination for any part of file read

      result = remaining<100?remaining:100; // -1 saving is for last null

      MyMemcpy(read_data, &lp_dir->data[ fd_array[ fd ].offset ], result);

      fd_array[fd].offset += result;  // advance our "current" ptr
   }
}

// check ownership of fd and the fd is valid within range
int FScanAccessFD( int fd, int owner ) {
   if( fd_array[fd].owner == owner) return 1;
   return 0;     // not good
}

// Search our (fixed size) table of file descriptors. returns fd_array[] index
// if an unused entry is found, else -1 if all in use. if avail, then all
// fields are initialized.
int FSallocFD( int owner ) {
   int i;

   for(i=0; i<FD_NUM; i++) {
      if( 0 == fd_array[i].owner ) {
         fd_array[i].owner = owner;
         fd_array[i].offset = 0;
         fd_array[i].item = 0;     // NULL is (void *)0, spede/stdlib.h

         return i;
      }
   }

   return -1;   // no free file descriptors
}

dir_t *FSfindName( char *name ) {
   dir_t *starting;

// assume every path relative to root directory. Eventually, the user
// context will contain a "current working directory" and we can possibly
// start our search there
   if( name[0] == '/' ) {
      starting = root_dir;

      while( name[0] == '/' ) name++;

      if( name[0] == 0 ) return root_dir; // client asked for "/"
   } else {
// path is relative, so start off at CWD for this process
// but we don't have env var CWD, so just use root as well
      starting = root_dir; // should be what env var CWD is
   }

   if( name[0] == 0 ) return 0;

   return FSfindNameSub(name, starting);
}

// go searching through a single dir for a name match. use MyStrcmp()
// for case-insensitive compare. use '/' to separate directory components
// if more after '/' and we matched a dir, recurse down there
// RETURN: ptr to dir entry if found, else 0
// once any dir matched, don't return name which dir was matched
dir_t *FSfindNameSub( char *name, dir_t *this_dir ) {
   dir_t *dir_p = this_dir;
   int len = MyStrlen(name);
   char *p;

// if name is '.../...,' we decend into subdir
   if( ( p = strchr( name, '/' ) ) != 0) len = p - name;  // p = to where / is (high mem)

   for( ; dir_p->name; dir_p++ ) {
//      if((unsigned int)dir_p->name > 0xdfffff) return 0; // tmp bug-fix patch

      if( 1 == MyStrcmp( name, dir_p->name, len ) ) {
         if( p && p[1] != 0 ) { // not ending with name, it's "name/..."
// user is trying for a sub-dir. if there are more components, make sure this
// is a dir. if name ends with "/" we don't check. thus "hello.html/" is legal
            while( *p == '/' ) {
               p++;                           // skipping trailing /'s in name
               if( '\0' == *p ) return dir_p; // name "xxx/////" is actually legal
            }

// altho name given is "xxx/yyy," xxx is not a directory
            if(dir_p->mode != MODE_DIR) return 0; // bug-fix patch for "cat h/in"

            name = p;
            return FSfindNameSub(name, (dir_t *)dir_p->data);
         }
         return dir_p;
      }
   }

   return 0;   // no match found
}

// copy what dir_p points to (dir_t) to what attr_p points to (attr_t)
void FSdir2attr( dir_t *dir_p, attr_t *attr_p ) {
   attr_p->dev = current_pid;            // current_pid manages this i-node

   attr_p->inode = dir_p->inode;
   attr_p->mode = dir_p->mode;
   attr_p->nlink = ( A_ISDIR( attr_p->mode ) ) + 1;
   attr_p->size = dir_p->size;
   attr_p->data = dir_p->data;
}

void FScloseHandler(void) {
   int fd;

   fd = pcb[current_pid].TF_p->eax;

   if (FScanAccessFD(fd, current_pid))fd_array[fd].owner = 0;
   else  cons_printf("FScloseHandler: cannot close FD!\n");
}
