// types.h, 159

#ifndef __TYPES_H__
#define __TYPES_H__

#include "FStypes.h"

#define LOOP 1666667         // handly loop limit exec asm("inb $0x80");
#define PROC_NUM 20          // max number of processes
#define Q_SIZE 20            // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes


#define PORT_NUM 3        // 3 extra serial ports: COM2/3/4
#define BUFF_SIZE 101     // 100 data char + delimiter null char
#define TIME_LIMIT 10   // Fast process switch scheduling

#define MEM_BASE 0xE00000    // memory pages start at 14M
#define MEM_PAGE_NUM 100     // kernel maintains 100 memory pages
#define MEM_PAGE_SIZE 4096   // a memory page has 4096 bytes

// Trapframe to save the state of CPU registers /before entering
// kernel code, and loaded back (in reverse) to resume process
typedef struct { 
	unsigned short	gs;      // 16-bit data seg registers below
	unsigned short	filler1; // filler, making 4-byte alignment
	unsigned short	fs; 
	unsigned short	filler2; 
	unsigned short	es; 
	unsigned short	filler3; 
	unsigned short	ds; 
	unsigned short	filler4; 
 
	unsigned int	edi; // PUSHA register state frame below
	unsigned int	esi; 
	unsigned int	ebp; 
	unsigned int	esp; // Push: before PUSHA, Pop: skipped 
	unsigned int	ebx; 
	unsigned int	edx; 
	unsigned int	ecx; 
	unsigned int	eax; 
 
	unsigned int	event_num; // indicate what event occurred
 
	unsigned int	eip; // processor state frame below
	unsigned int	cs; 
	unsigned int	eflags; 
} TF_t;  // trapframe type
 
typedef void (*func_ptr_t)(); // void-return function pointer type

// this is the same as constant defines: FREE=0, RUN=1, etc.
typedef enum {FREE, RUN, READY, SLEEP, WAIT, ZOMBIE} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int cpu_time;             // CPU runtime
   TF_t *TF_p;               // points to trapframe of process
   int wake_time;	     // when to wake up
   int ppid;
} pcb_t;

typedef struct {             // generic queue type
   int q[Q_SIZE];            // integers are queued in q[] array
   int size;                 // size is also where the tail is for new data
} q_t;

typedef struct {
	int owner;				// owner PID
	int passes;				// max # of processes allowed to pass this semaphore
	q_t wait_q;  			// blocked PID's
} sem_t;

typedef struct {
   int owner,                // allocation mark
       IO,                   // I/O map #
       write_sid,            // write flow-control sid
       read_sid,             // read flow-control sid
       write_ok;             // write available flag
   q_t write_q,              // write buffer
       read_q,               // read buffer
       loopback_q;           // loopback buffer
} port_t;

typedef struct {
   int owner,
   char *addr;
} mem_page_t;

#endif //__TYPES_H__
