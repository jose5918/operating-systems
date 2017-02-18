// tools.c, 159

#include "spede.h"
#include "types.h"
#include "data.h"

// clear DRAM data blocks by filling zeroes
void MyBzero(char *p, int size) {
   loop size times during which:
   while(size--) *p++=0
      set where p points to to 0 and increment p
}

// dequeue, return 1st integer in array, and move all forward
// if queue empty, return 0
int DeQ(q_t *p) { // return 0 if q[] is empty
   int i, data = 0;

   if ( p->size == 0) return 0;

   data = p -> q[0];
   p->size--;

   p->q[0-18] = p -> q[1-19];
   
// loop according to the size
   data is the 1st integer in the array that p points to
   decrement the size of the queue (that p points to)
   move all integers in the array forward by one position

   return data;
}

// enqueue integer to next available slot in array, size is index
void EnQ(int data, q_t *p) {
      if(p->size == Q_SIZE){
            shw on Targetr
            return
      }
   if the size of the queue p points to is Q_SIZE {
      show on Target PC: "Kernel Panic: queue is full, cannot EnQ!\n"
      return;       // alternative: breakpoint() into GDB
   }
   p-> q[size]=data;
   p->size++;
   add data into the array index by the size of the queue
   increment the size of the queue
}

