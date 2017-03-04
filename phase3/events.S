// events.S, initial handling sequence of an event, build TF mainly

#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#include "events.h"                 // for K_DATA, K_STACK_SIZE below

// set CPU DS and ES registers to K_DATA (data segment # registers)
#define SET_K_SEGS movw $(K_DATA), %ax; mov %ax, %ds; mov %ax, %es

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (global)
.text

// loads the trapframe of the selected process in order to run it
ENTRY(Loader)  // Loader(pcb[current_pid].TF_p)
    movl 4(%esp), %eax    // copy what's above stack pointer to eax
    movl %eax, %esp        // copy eax to esp (point esp to TF)
    popl %gs               // 1st pop gs (see TF_t in types.h)
    popl %fs               // then fs
    popl %es               // then es
    popl %ds               // then ds
    popa                   // pop all: e-d/s-i, e-b/s-p, e-b/d/c/a-x
    addl $4, %esp          // skip 4 bytes (event_num)
    iret                   // interrupt return (pop eip, cs, eflags)

ENTRY(GetPidEvent)
		pushl $GETPID_EVENT
		jmp CommonPart

ENTRY(SleepEvent)
		pushl $SLEEP_EVENT
		jmp CommonPart		
   
ENTRY(TimerEvent)
    pushl $TIMER_EVENT // push event_num
    jmp CommonPart

ENTRY(SemAllocEvent)
    pushl $SEMALLOC_EVENT // push event_num
    jmp CommonPart

ENTRY(SemWaitEvent)
    pushl $SEMWAIT_EVENT // push event_num
    jmp CommonPart

ENTRY(SemPostEvent)
    pushl $SEMPOST_EVENT // push event_num
    jmp CommonPart

CommonPart:
    pusha              // push all: e-a/c/d/b-x, e-s/b-p, e-s/d-i
    pushl %ds          // push 4 data seg registers
    pushl %es
    pushl %fs
    pushl %gs 
    movl %esp, %ecx    // copy TF addr (esp) to ecx
    cld                // clear direction flag bit
    SET_K_SEGS           // set kernel data segment registers
    leal k_stack + K_STACK_SIZE, %esp // point esp to kernel stack
    pushl %ecx          // push ecx (TF addr) there
    call CNAME(Kernel)  // call Kernel code
