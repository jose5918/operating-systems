// events.S, initial handling sequence of an event, build TF mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

// set CPU DS and ES registers to K_DATA (data segment # registers)
#define SET_K_SEGS movw $(K_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (global)
.text

// loads the trapframe of the selected process in order to run it
ENTRY(Loader)  // Loader(pcb[current_pid].TF_p)
    movl 4(%esp), %eax;    // copy what's above stack pointer to eax
    movl %eax, %esp  // copy eax to esp (point esp to TF)
    popl %gs
    popl %fs 
    popl %es 
    popl %ds
    popa 
    addl $4, %esp
    iret

   // 1st pop gs (see TF_t in types.h)
   // then fs
   // then es
   // then ds
   // pop all: e-d/s-i, e-b/s-p, e-b/d/c/a-x
   // skip 4 bytes (event_num)
   // interrupt return (pop eip, cs, eflags)

    #Below ENTRY(TimerEvent)
    pushl $TIMER_EVENT
    pusha
    pushl %ds
    pushl /es/fs/gs
    movl %esp, %ecx
    cld
    SET_K_SEGS
    leal k_stack + K_STACK_SIZE, %esp
    pushl %ecx
    call CNAME(Kernel)

// push event_num then go into kernel
ENTRY(TimerEvent)           // push eflags, cs, eip (by circuit)
   // push event_num
   // push all: e-a/c/d/b-x, e-s/b-p, e-s/d-i
   // push 4 data seg registers
   // copy TF addr (esp) to ecx
   // clear direction flag bit
   SET_K_SEGS               // set kernel data segment registers
   leal k_stack + K_STACK_SIZE, %esp  // point esp to kernel stack
   // push ecx (TF addr) there
   // call Kernel code

