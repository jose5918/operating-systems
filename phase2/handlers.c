// handlers.c, 159

#include "spede.h"
#include "types.h"
#include "handlers.h"
#include "tools.h"
#include "data.h"
#include "proc.h"

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
  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  // Clear the runtime stack
  MyBzero((char *)&pcb[pid], PROC_STACK_SIZE);
  // Process state now READY
  pcb[pid].state = READY;
  EnQ(pid, &ready_q);

  // point TF_p to highest area in the stack (but has space for a TF)
  pcb[pid].TF_p = (TF_t *)&proc_stack[pid][PROC_STACK_SIZE];
  pcb[pid].TF_p--;
  pcb[pid].TF_p->eip = (unsigned int)p;
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
	int i;
  pcb[current_pid].cpu_time++;
	current_time++;
  // not too sure about things under this
	for(i = 0; i < PROC_NUM ; i++){
		if(pcb[i].state == SLEEP && pcb[i].wake_time == current_time){
			pcb[i].state = READY;		
			EnQ(i, &ready_q);
		}
	}			
   


	if (pcb[current_pid].cpu_time == TIME_LIMIT) {
    pcb[current_pid].state = READY;
    EnQ(current_pid, &ready_q);
    current_pid = 0;
  }
  // while size of q in not empty

	
  outportb(0x20, 0x60);
}

void GetPidHandler(void) {
		pcb[current_pid].TF_p->eax = current_pid; 
		outportb(0x20, 0x60);
}

void SleepHandler(void) {
		pcb[current_pid].wake_time = current_time + (100*pcb[current_pid].TF_p->eax);
		pcb[current_pid].state = SLEEP;
		current_pid = 0;
		outportb(0x20, 0x60);
}
