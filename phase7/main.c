//main.c, 159
//
// Team Name: Clang (Members: Jose Aguirre and Ahriben Gonzalez)

#include "spede.h"
#include "handlers.h"
#include "tools.h"
#include "proc.h"
#include "types.h"
#include "events.h"
#include "FSdata.h"


int current_pid;
q_t ready_q, free_q;
pcb_t pcb[PROC_NUM];
char proc_stack[PROC_NUM][PROC_STACK_SIZE];
int current_time;

sem_t sem[Q_SIZE];
unsigned short *ch_p = (unsigned short *) 0xB8000;
int vehicle_sid;

port_t port[PORT_NUM];

mem_page_t mem_page[MEM_PAGE_NUM];

void Scheduler() {
  if (current_pid != 0) return;

  if (ready_q.size == 0) {
    cons_printf("Kernel Panic: no process to run!\n");
    breakpoint();
  }

  current_pid = DeQ(&ready_q);
  pcb[current_pid].state = RUN;
  ch_p[current_pid*80+43] =0xf00 + 'R';
  pcb[current_pid].cpu_time = 0;
}


int main() {
  int i;

  struct i386_gate *IDT_p;

 current_time = 0;
 current_pid = 0;
  vehicle_sid =-1;

  MyBzero((char *)&free_q, sizeof(q_t));
  MyBzero((char *)&ready_q, sizeof(q_t));
  MyBzero((char *)&sem[0], (sizeof(sem_t)*Q_SIZE));


  for (i = 1; i < Q_SIZE; i++) {
    EnQ(i, &free_q);
  }

  IDT_p = get_idt_base();
  cons_printf("IDT located at DRAM addr %x (%d).\n", IDT_p, IDT_p);

  fill_gate(&IDT_p[TIMER_EVENT], (int)TimerEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[SLEEP_EVENT], (int)SleepEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[GETPID_EVENT], (int)GetPidEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[SEMALLOC_EVENT], (int)SemAllocEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[SEMWAIT_EVENT], (int)SemWaitEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[SEMPOST_EVENT], (int)SemPostEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[SYSPRINT_EVENT], (int)SysPrintEvent, get_cs(), ACC_INTR_GATE, 0);
 
 fill_gate(&IDT_p[PORT_EVENT], (int)PortEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[PORTALLOC_EVENT], (int)PortAllocEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[PORTWRITE_EVENT], (int)PortWriteEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[PORTREAD_EVENT], (int)PortReadEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[PORT_EVENT+1], (int)PortEvent, get_cs(), ACC_INTR_GATE, 0);

 // phase6
 fill_gate(&IDT_p[FSFIND_EVENT], (int)FSfindEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[FSOPEN_EVENT], (int)FSopenEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[FSREAD_EVENT], (int)FSreadEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[FSCLOSE_EVENT], (int)FScloseEvent, get_cs(), ACC_INTR_GATE, 0);

 //phase 7
 fill_gate(&IDT_p[FORK_EVENT], (int)ForkEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[WAIT_EVENT], (int)WaitEvent, get_cs(), ACC_INTR_GATE, 0);
 fill_gate(&IDT_p[EXIT_EVENT], (int)ExitEvent, get_cs(), ACC_INTR_GATE, 0);

  for(i = 0; i<FD_NUM-1; i++){
    fd_array[i].owner = 0;
  }

  for(i = 0; i<MEM_PAGE_NUM; i++){
    mem_page[i].owner = 0;
    mem_page[i].addr = (char *)(MEM_BASE + (i*MEM_PAGE_SIZE));
  }

  root_dir[0].size = sizeof(root_dir);   // can only be assigned during runtime
  bin_dir[0].size = sizeof(bin_dir);     // even tho they're compiler-time sizes
  bin_dir[1].size = root_dir[0].size;    // otherwise, they would be recursive
  www_dir[0].size = sizeof(www_dir);     // definitions which compiler rejects
  www_dir[1].size = root_dir[0].size;

  outportb(0x21, ~0x19);

  NewProcHandler(Init);
  NewProcHandler(TermProc);
  NewProcHandler(TermProc);
  for(i = 0; i<PORT_NUM; i++){
    port[i].owner = 0;
  }

  Scheduler();

  Loader(pcb[current_pid].TF_p);
  return 0;
}

void Kernel(TF_t *TF_p) {

  pcb[current_pid].TF_p = TF_p;

  switch (TF_p->event_num) {
    case TIMER_EVENT:
      TimerHandler();
      break;
    case PORT_EVENT:
      PortHandler();
      break;
    case PORTWRITE_EVENT:
      PortWriteHandler((char)TF_p->eax,TF_p->ebx);
      break;
    case PORTREAD_EVENT:
      PortReadHandler((char *)TF_p->eax,TF_p->ebx);
      break;
    case PORTALLOC_EVENT:
      PortAllocHandler(&(TF_p->eax));
      break;
    case FSREAD_EVENT:
      FSreadHandler();
      break;
    case FSFIND_EVENT:
      FSfindHandler();
      break;
    case FSOPEN_EVENT:
      FSopenHandler();
      break;
    case FSCLOSE_EVENT:
      FScloseHandler();
      break;
    case SYSPRINT_EVENT:
      SysPrintHandler((char *)TF_p->eax);
      break;
    case SEMALLOC_EVENT:
      SemAllocHandler(TF_p->eax);
      break;
    case SEMWAIT_EVENT:
      SemWaitHandler(TF_p->eax);
      break;
    case SEMPOST_EVENT:
      SemPostHandler(TF_p->eax);
      break;
    case GETPID_EVENT:
      GetPidHandler();
      break;
    case SLEEP_EVENT:
      SleepHandler();
      break;
    case FORK_EVENT:
      ForkHandler((char *)TF_p->eax, &(TF_p->ebx));
      break;
    case WAIT_EVENT:
      WaitHandler(&(TF_p->eax));
      break;
    case EXIT_EVENT:
      ExitHandler(TF_p->eax);
      break;
    default:
      cons_printf("Kernel Panic: Unknown event_num %d!", TF_p->event_num);
      breakpoint();
  }

  Scheduler();
  Loader(pcb[current_pid].TF_p);
}
