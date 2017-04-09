#include "spede.h"
#include "types.h"
#include "data.h"

void MyBzero(char *p, int size) {
  while (size--) *p++ = 0;
}

int DeQ(q_t *p) {
  int i, data = 0;
  if (p->size == 0) return 0;
  data = p->q[0];
  p->size--;
  for (i = 0; i < p->size; i++) {
    p->q[i] = p->q[i + 1];
  }
  return data;
}


void EnQ(int data, q_t *p) {
  if (p->size == Q_SIZE) {
    cons_printf("Kernel Panic: queue is full, cannot EnQ!\n");
    return;
  }

  p->q[p->size] = data;
  p->size++;
}
