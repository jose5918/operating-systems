// handlers.c, 159

#include "handlers.h"
#include "data.h"
#include "proc.h"
#include "spede.h"
#include "tools.h"
#include "types.h"

// to create process, alloc PID, PCB, and stack space
// build TF into stack, set PCB, register PID to ready_q
void NewProcHandler(func_ptr_t p) {  // arg: where process code starts
  int pid;

  if (free_q.size == 0) {  // this may occur for testing
    cons_printf("Kernel Panie: no more PID left!\n");
    return;  // alternative breakpoint into GDB
  }
  // Get 'pid' from free_q
  pid = DeQ(&free_q);
  // Clear the process controll block
  MyBzero(&pcb[pid], sizeof(pcb_t));
  // Clear the runtime stack
  MyBzero(&pcb[pid], PROC_STACK_SIZE);
  // Process state now READY
  pcb[pid].state = READY;
  EnQ(pid, &ready_q);

  // point TF_p to highest area in the stack (but has space for a TF)
  pcb[pid].TF_p = (TF_t *)&proc_stack[pid].[PROC_STACK_SIZE];
  pcb[pid].TF_p->eip = (int)p;
  pcb[pid].TF_p->eflags = EF_DEFAULT_VALUE | EF_INTR;

  // functions from spede/machine/proc_reg.h
  pcb[pid].TF_p->cs = get_cs();  // duplicate from current CPU
  pcb[pid].TF_p->ds = get_ds();
  pcb[pid].TF_p->es = get_es();
  pcb[pid].TF_p->fs = get_fs();
  pcb[pid].TF_p->gs = get_gs();
}

// count cpu_time of running process and preempt it if reaching limit
void TimerHandler(void) {
  pcb[current_pid].cpu_time++;
  // not too sure about things under this
  if (pcb[current_pid].cpu_time == TIME_LIMIT) {
    pcb[current_pid].state = READY;
    EnQ(current_pid, &ready_q);
    current_pid = 0;
  }

  Don't forget: notify PIC event-handling done 
}
