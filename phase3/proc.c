// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "spede.h"  // cons_printf below needs
#include "data.h"   // current_pid needed below
#include "proc.h"   // prototypes of processes
#include "services.h" 


// Init PID 1, always ready to run, never preempted
void Init(void) {
  int i;

  while (1) {
    //cons_printf("1..");  // muted
    for (i = 0; i < LOOP; i++) {  // to cause approx 1 second of delay
      asm("inb $0x80");
    }
  }
}

// PID 2, 3, 4, etc. mimicking a usual user process
void UserProc(void) {
  while (1) {
    cons_printf("%d..", GetPid());  // will change to GetPID later
		Sleep(GetPid());  
  }
}

void Vehicle(void) {             // phase3 tester (multiple processes)
   int i, my_pid;

   if(vehicle_sid == -1) vehicle_sid = SemAlloc(3); // max passes 3
   my_pid = GetPid();

   while(1) {
      ch_p[my_pid*80+45] = 0xf00 + 'f';     // show I'm off the bridge
      for(i=0; i<LOOP; i++) asm("inb $0x80"); // spend a sec in RUN state
      SemWait(vehicle_sid);                 // ask for a pass
      ch_p[my_pid*80+45] = 0xf00 + 'o';     // show I'm on the bridge
      Sleep(1);                             // a sec SLEEP state (holding pass)
      SemPost(vehicle_sid);                 // return the pass
   }
}