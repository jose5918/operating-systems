// main.c, 159
// this is kernel code for phase 1
//
// Team Name: ??????? (Members: ?????? and ??????)

#include "events.h"    // events for kernel to serve
#include "handlers.h"  // handler code
#include "proc.h"      // processes such as Init()
#include "spede.h"     // given SPEDE stuff
#include "tools.h"     // small functions for handlers
#include "types.h"     // data types

// kernel's own data:
int current_pid;      // current selected PID; if 0, none selected
q_t ready_q, free_q;  // processes ready to run and not used
pcb_t pcb[PROC_NUM];  // process control blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE];  // process runtime stacks

void Scheduler() {               // choose a PID as current_pid to load/run
  if (current_pid != 0) return;  // if continue below, find one for current_pid

  if (ready_q.size == 0) {
    cons_printf("Kernel Panic: no process to run!\n");  // big problem!
    breakpoint();
  }

  current_pid = DeQ(&ready_q) pcb[current_pid].state = RUN;  // from RUN
  cpu_time = 0;
}

// OS bootstrap from main() which is process 0, so we do not use this PID
int main() {
  int i;
  struct i386_gate *IDT_p;  // DRAM location where IDT is

  // use tool function MyBzero to clear the two PID queues
  MyBzero(&free_q, sizeof(q_t));
  MyBzero(&ready_q, sizeof(q_t))

      // queue free queue with PID 1~19
      for (i = 1; i < Q_SIZE; i++) {
    EnQ(i, &free_q);
  }
  // init IDT_p
  IDT_p = get_idt_base();
  cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p);
  // set IDT entry 32 like our timer lab  // fillgate
  fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
  // set PIC mask to open up for timer IRQ0 only // outport
  outportb(0x21, ~1);

  // Not sure about under here

  // call NewProcHandler(Init) to create Init proc
  // Init passed as function pointer
  NewProcHandler(Init);
  // call Scheduler() to select current_pid (will be 1)
  current_pid = 1;  // shouldn't this be 0?
                    // or maybe scheduler should check for 1
                    // or I might just be crazy
  Scheduler();
  // call Loader with the TF address of current_pid
  Loader(pcb[current_pid].TF_p);
  return 0;
}

void Kernel(TF_t *TF_p) {  // kernel code exec (at least 100 times/second)
  char key;

  pcb[current_pid].TF_p = TF_p;
  // switch according to the event_num in the TF TF_p points to
  switch (TF_p->event_num) {
    case TIMER_EVENT:
      TimerEventHandler();
      break;
    default:
      cons_printf("Kernel Panic: Unknown event_num %d!", TF_p->event_num);
      breakpoint();
  }

  if (cons_kbhit()) {
    key = cons_getchar();
    switch (key) {
      case 'n':
        NewProcHandler(UserProc);
        break;
      case 'b':
        breakpoint();
    }
  }
  Scheduler();
  // call loader with the TF address of current_pid
  Loader(pcb[current_pid].TF_p);
}
