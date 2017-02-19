// tools.c, 159

#include "data.h"
#include "spede.h"
#include "types.h"

// clear DRAM data blocks by filling zeroes
void MyBzero(char *p, int size) {
  // This will loop size times
  // Set where p = 0 and increment p
  while (size--) *p++ = 0;
}

// dequeue, return 1st integer in array, and move all forward
// if queue empty, return 0
int DeQ(q_t *p) {  // return 0 if q[] is empty
  int i, data = 0;

  // if the size of the queue p points to is 0, return data (which is 0 anyway)
  if (p->size == 0) return 0;
  // data is the 1st integer in the array that p points to
  data = p->q[0];
  // decrement the size of the queue
  p->size--;
  for (i = 0; i < Q_SIZE - 1; i++) {
    p->q[i] = p->q[i + 1];  // p->q[0 - 18] = p->q[1 - 19];
  }

  return data;
}

// enqueue integer to next available slot in array, size is index
void EnQ(int data, q_t *p) {
  // if the size of the queue p points to is Q_SIZE, return
  if (p->size == Q_SIZE) {
    cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
    return;
  }
  // add data into the array index by the size of the queue
  p->q[size] = data;
  // increment the size of the queue
  p->size++;
}
