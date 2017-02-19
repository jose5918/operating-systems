// Done (I think)
// proc.c, 159
// all processes are coded here
// processes do not use kernel space (data.h) or code (handlers, tools, etc.)
// all must be done thru system service calls

#include "proc.h"   // prototypes of processes
#include "data.h"   // current_pid needed below
#include "spede.h"  // cons_printf below needs

// Init PID 1, always ready to run, never preempted
void Init(void) {
  int i;

  while (1) {
    cons_printf("1..\n");
    for (i = 0; i < LOOP; i++) {  // to cause approx 1 second of delay
      asm("inb $0x80");
    }
  }
}

// PID 2, 3, 4, etc. mimicking a usual user process
void UserProc(void) {
  int i;

  while (1) {
    cons_printf("%d..\n", current_pid);  // will change to GetPID later
    for (i = 0; i < LOOP; i++) {         // to cause approx 1 second of delay
      asm("inb $0x80");
    }
  }
}
