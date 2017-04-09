#include "spede.h"
#include "types.h"
#include "handlers.h"
#include "tools.h"
#include "data.h"
#include "proc.h"


void NewProcHandler(func_ptr_t p) {
  int pid;

  if (free_q.size == 0) {
    cons_printf("Kernel Panie: no more PID left!\n");
    return;
  }

  pid = DeQ(&free_q);

  MyBzero((char *)&pcb[pid], sizeof(pcb_t));
  MyBzero((char *)&pcb[pid], PROC_STACK_SIZE);

  pcb[pid].state = READY;
  if (current_pid != 0){
    ch_p[current_pid*80+43] =0xf00 + 'r';
  }
  EnQ(pid, &ready_q);


  pcb[pid].TF_p = (TF_t *)&proc_stack[pid][PROC_STACK_SIZE];
  pcb[pid].TF_p--;
  pcb[pid].TF_p->eip = (unsigned int)p;
  pcb[pid].TF_p->eflags = EF_DEFAULT_VALUE | EF_INTR;
  pcb[pid].TF_p->cs = get_cs();
  pcb[pid].TF_p->ds = get_ds();
  pcb[pid].TF_p->es = get_es();
  pcb[pid].TF_p->fs = get_fs();
  pcb[pid].TF_p->gs = get_gs();
}


void TimerHandler(void) {
 int i;
  pcb[current_pid].cpu_time++;
 current_time++;

 for(i = 0; i < PROC_NUM ; i++){
  if(pcb[i].state == SLEEP && pcb[i].wake_time == current_time){
   pcb[i].state = READY;
      ch_p[current_pid*80+43] =0xf00 + 'r';
   EnQ(i, &ready_q);
  }
 }

 if (pcb[current_pid].cpu_time == TIME_LIMIT) {
    pcb[current_pid].state = READY;
    ch_p[current_pid*80+43] =0xf00 + 'r';
    EnQ(current_pid, &ready_q);
    current_pid = 0;
  }

  outportb(0x20, 0x60);
}

void GetPidHandler(void) {
  pcb[current_pid].TF_p->eax = current_pid;
}

void SleepHandler(void) {
  pcb[current_pid].wake_time = current_time + (100*pcb[current_pid].TF_p->eax);
  pcb[current_pid].state = SLEEP;
    ch_p[current_pid*80+43] =0xf00 + 'S';
  current_pid = 0;
}

void SemAllocHandler(int passes){
    int i;
    for(i=0; i < Q_SIZE; i++){
      if (sem[i].owner==0){
        MyBzero((char *)&sem[i], sizeof(sem_t));
        sem[i].owner = current_pid;
        sem[i].passes=passes;
        ch_p[51] =0xf00 + passes + '0';
        pcb[current_pid].TF_p->ebx = i;
        break;
      }
    }
}

void SemWaitHandler(int sid){
    if (sem[sid].passes>0){
      sem[sid].passes--;
      ch_p[51] =0xf00 + sem[sid].passes + '0';
      return;
    }
    EnQ(current_pid, &(sem[sid].wait_q));
    pcb[current_pid].state = WAIT;
    ch_p[current_pid*80+43] =0xf00 + 'W';
    current_pid = 0;

}

void SemPostHandler(int sid){
    int process_number;
    if (sem[sid].wait_q.size > 0){
      process_number=DeQ(&(sem[sid].wait_q));
      pcb[process_number].state = READY;
      ch_p[current_pid*80+43] =0xf00 + 'r';
      EnQ(process_number,&ready_q);
      return;
    }
    sem[sid].passes++;
    ch_p[51] =0xf00 + sem[sid].passes + '0';
}

void SysPrintHandler(char *p){
    int i, code;

    const int printer_port = 0x378;
    const int printer_data = printer_port + 0;
    const int printer_status = printer_port + 1;
    const int printer_control = printer_port + 2;

    outportb(printer_control, 16);
    code = inportb(printer_status);
    for(i=0; i<50; i++) asm("inb $0x80");
    outportb(printer_control, 4 | 8 );

    while(*p) {
        outportb(printer_data, *p);
        code = inportb(printer_control);
        outportb(printer_control, code | 1);
        for(i=0; i<50; i++) asm("inb $0x80");
        outportb(printer_control, code);

        for(i = 0; i < LOOP*3; i++) {
          code = inportb(printer_status) & 64;
          if(code == 0) break;
          asm("inb $0x80");
        }

        if(i == LOOP*3) {
          cons_printf(">>> Printer timed out!\n");
          break;
        }

        p++;
    }
}
