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
	asm("movl %0, %%eax ;int $101"
		:
		:"g" (seconds)
		:"%eax");
}

int SemAlloc(int passes){
	int sid;
	asm("movl %1, %%eax;        // save register EAX to stack
           int $102;           // interrupt CPU with IDT event 100
           movl %%ebx, %0;     // after, copy EAX to pid (%0 is 1st below)
	  popl %%eax;
	  popl %%ebx"
          : "=g" (sid)         // one output item
          : "g"  (passes)                  // no input items
          );
      return sid;
}

void SemWait(int sid){
      asm("movl %0,  %%eax;        // save register EAX to stack
           int $103;          // interrupt CPU with IDT event 100
	   popl %%eax"
          :			         // one output item
          : "g" (sid)                    // no input items
          );
}

void SemPost(int sid){
      asm("movl %0, %%eax;        // save register EAX to stack
           int $104;           // interrupt CPU with IDT event 100
	   popl %%eax"
          : 		         // one output item
          : "g" (sid)                   // no input items
          );
}

