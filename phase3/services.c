// This can be deleted for now
// It is for phase 2
int GetPid(void) {  // function receives no arguments, but return an integer
      int pid;
      asm("pushl %%eax;        // save register EAX to stack
           int $100;           // interrupt CPU with IDT event 100
           movl %%eax, %0;     // after, copy EAX to pid (%0 is 1st below)
           popl %%eax"         // restore EAX from stack
          : "=g" (pid)         // one output item
          :                    // no input items
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
