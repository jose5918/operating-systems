int GetPid(void) {  
      int pid;
      asm("pushl %%eax;
           int $100;        
           movl %%eax, %0;
           popl %%eax"        
          : "=g" (pid)         
          :                    
          );
      return pid;
   }

void Sleep(int seconds) {
	asm("pushl %%eax;
         movl %0, %%eax;
         int $101;  
         popl %%eax;"
        :
		:"g" (seconds)
        );
}

int SemAlloc(int passes) {
    int sid;
    asm("pushl %%eax; pushl %%ebx;
         movl %1, %%eax;
         int $102;
         movl %%ebx, %0;
         popl %%ebx; popl %%eax"
         : "=g" (sid)
         : "g" (passes)
         );
    return sid;
}

void SemWait(int sid) {
	asm("pushl %%eax;
         movl %0, %%eax;
         int $103;  
         popl %%eax;"
        :
		:"g" (sid)
        );
}

void SemPost(int sid) {
	asm("pushl %%eax;
         movl %0, %%eax;
         int $104;  
         popl %%eax;"
        :
		:"g" (sid)
        );
}

void SysPrint(char *p) {
	asm("pushl %%eax;
         movl %0, %%eax;
         int $105;
         popl %%eax;"
        :
		:"g" ((int)p)
        );
}

int PortAlloc(void) {
	int port_num;
	asm("pushl %%eax;
        int $106;
	movl %%eax, %0;
        popl %%eax;"
        :"=g" (port_num)
	:
        );
      Sleep(1);//after getting port_num, Sleep for a second
      // call SemAlloc(?) to allocate for write_sid in port data allocated
      port[port_num].write_sid = SemAlloc(Q_SIZE);
      // call SemAlloc(?) to allocate for read_sid in port data allocated
      port[port_num].read_sid = SemAlloc(0);
      //***** set size of read_q to 0 (KB may have bufferred extra char as space bar pressed)
      port[port_num].read_q.size = 0;
      return port_num;
}


void PortWrite(char *p, int port_num) {
	// loop thru string: sem-wait and then call "int xxx" to write a char
     while(*p){
	//loop while what p points to has something to print (not null)
	//SemWait(?) for checking if the write buffer has space
	SemWait(port[port_num].write_sid);
         asm("pushl %%eax;
              pushl %%ebx;
              movl %0, %%eax;
              movl %1, %%ebx;
              int $107;
              popl %%ebx;
              popl %%eax;"
             : 
             : "g" ((int)p), "g"(port_num)
             );
	p++; //advance the address in p to next character
     }
}

void PortRead(char *p, int port_num) { // to read terminal KB
      int size;
      // loop: sem-wait then "int xxx" to get a char until \r (or size limit)
      size = 0;// set size to 0
      while(11 {             // break at \r or at BUFF_SIZE-1
         SemWait(port[port_num].read_sid);      // flow-control
         asm("pushl %%eax;
              pushl %%ebx;
              movl %0, %%eax;
              movl %1, %%ebx;
              int $108;
              popl %%ebx;
              popl %%eax;"
             :
             : "g" ((int)p), "g" (port_num) // two inputs: p (addr for getting a char) and port #
             );
         //break loop if what p points to is filled out with '\r'
         if (*p == '\r') break;
	 p++; //advance addr p to next character
         size++; //increment size by 1
         //break loop if size reaching BUFF_SIZE-1
         if (size == BUFF_SIZE-1) break;
	}
	*p = '\0';  // null-terminate str, overwirte \r
}
