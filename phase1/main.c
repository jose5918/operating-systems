// main.c, 159
// this is kernel code for phase 1
//
// Team Name: ??????? (Members: ?????? and ??????)

#include "spede.h"      // given SPEDE stuff
#include "handlers.h"   // handler code
#include "tools.h"      // small functions for handlers
#include "proc.h"       // processes such as Init()
#include "types.h"      // data types
#include "events.h"     // events for kernel to serve

// kernel's own data:
int current_pid;        // current selected PID; if 0, none selected
q_t ready_q, free_q;    // processes ready to run and not used
pcb_t pcb[PROC_NUM];    // process control blocks
char proc_stack[PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

void Scheduler() {                 // choose a PID as current_pid to load/run
   if (current_pid!= 0, return // if continue below, find one for current_pid

   if ready_q.size is 0 {
      show on Target PC: "Kernel Panic: no process to run!\n" // big problem!
      breakpoint();
   }

   current_pid=DeQ(&ready_q)
   pcb[current_pid].state = RUN; // from RUN
   cpu_time=0;
}

// OS bootstrap from main() which is process 0, so we do not use this PID
int main() {
   int i;
   struct i386_gate *IDT_p; // DRAM location where IDT is

   use tool function MyBzero to clear the two PID queues
   MyBzero(&free_q,sizeof(q_t));
   MyBzero(&ready_q,sizeof())

   EnQ(0-19, &free_q);
   queue free queue with PID 1~19

   IDT_p = get_idt_base();
   init IDT_p (locate IDT location)
   show location on Target PC: "IDT located at DRAM addr %x (%d).\n"
   set IDT entry 32 like our timer lab  // fillgate
   set PIC mask to open up for timer IRQ0 only // outport

   call NewProcHandler(Init) to create Init proc
   call Scheduler() to select current_pid (will be 1)
   call Loader with the TF address of current_pid

   return 0; // compiler needs for syntax altho this statement is never exec
}

void Kernel(TF_t *TF_p) {   // kernel code exec (at least 100 times/second)
   char key;

   pcb[current_pid].TF_p =TF_p;
   switch (TF_p -> event_num) {
     TimerEvent:
      TimerEventHandler()
      break
    default:
      cons_printf(kernel panic)
      breakpoint
   }
   save TF_P into the PCB of current_pid

   switch according to the event_num in the TF TF_p points to {
      if it's timer event
         call timer event handler
         and break
      default of switch:
         show on Target PC "Kernel Panic: unknown event_num %d!\n"
         go into GDB
   }


   if a key is pressed on Target PC {
      get the key

      switch by the key obtained {
         if it's 'n'
            call NewProcHandler to create UserProc // NewProcHandler(UserProc); break;
            and break;
         if it's 'b'
            go into GDB
     }
   }

   call Scheduler() to select current_pid (if needed)
   call Loader with the TF address of current_pid
}

