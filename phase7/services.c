#include "data.h"

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
      Sleep(1);
      port[port_num].write_sid = SemAlloc(Q_SIZE);
      port[port_num].read_sid = SemAlloc(0);
      port[port_num].read_q.size = 0;
      return port_num;
}


void PortWrite(char *p, int port_num) {
	// loop thru string: sem-wait and then call "int xxx" to write a char
     while(*p){
	     SemWait(port[port_num].write_sid);
         asm("pushl %%eax;
              pushl %%ebx;
              movl %0, %%eax;
              movl %1, %%ebx;
              int $107;
              popl %%ebx;
              popl %%eax;"
             : 
             : "g" ((int)*p), "g"(port_num)
             );
	     p++; 
     }
}

void PortRead(char *p, int port_num) { // to read terminal KB
      int size;
      // loop: sem-wait then "int xxx" to get a char until \r (or size limit)
      size = 0;
      while(1) {             
         SemWait(port[port_num].read_sid);      
         asm("pushl %%eax;
              pushl %%ebx;
              movl %0, %%eax;
              movl %1, %%ebx;
              int $108;
              popl %%ebx;
              popl %%eax;"
             :
             : "g" ((int)p), "g" (port_num) 
             );
         if (*p == '\r') break;
	     p++; 
         size++; 
         if (size == BUFF_SIZE-1) break;
	}
	*p = '\0';  // null-terminate str, overwirte \r
}

void FSfind(char *name, char *cwd, char *data) {
    char tmp[BUFF_SIZE];
    MyStrcpy(tmp, cwd);
    MyStrcat(tmp, name);
    asm("pushl %%eax;
         pushl %%ebx;
         movl %0, %%eax;
         movl %1, %%ebx;
         int $109;
         popl %%ebx;
         popl %%eax;"
        :
        : "g" ((int)tmp), "g" ((int)data)
        );
}

int FSopen(char *name, char *cwd){
    char tmp[BUFF_SIZE];
    int fd_num;
    MyStrcpy(tmp, cwd);
    MyStrcat(tmp, name);
    asm("pushl %%eax;
         pushl %%ebx;
         movl %1, %%eax;
         int $110;
         movl %%ebx, %0;
         popl %%ebx;
         popl %%eax;"
        : "=g" (fd_num)
        : "g" ((int)tmp)
    );
    return fd_num;
}


void FSread(int fd, char *data){
    asm("pushl %%eax;
         pushl %%ebx;
         movl %0, %%eax;
         movl %1, %%ebx;
         int $111;
         popl %%ebx;
         popl %%eax;"
        :
        : "g" (fd), "g" ((int)data)
        );
}

void FSclose(int fd){
    asm("pushl %%eax;
         movl %0, %%eax;
         int $112;
         popl %%eax;"
        :
        : "g" (fd)
        );
}

int Fork(char *p){
    int child_pid;
    asm("pushl %%eax;
         pushl %%ebx;
         movl %1, %%eax;
         int $113;
         movl %%ebx, %0;
         popl %%ebx;
         popl %%eax;"
         : "=g" (child_pid)
         : "g" ((int)p)
    );
    return child_pid;
}

int Wait(void){
    int exit_num;
    asm("pushl %%eax;
         int $114;        
         movl %%eax, %0;
         popl %%eax"        
         : "=g" (exit_num)         
         :                    
    );
    return exit_num;

}

void Exit(int exit_num){
	asm("pushl %%eax;
         movl %0, %%eax;
         int $115;  
         popl %%eax;"
        :
		:"g" (exit_num)
        );
}
