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
int MyStrlen(char *p){
	int len = 0;
	while( p[len++] != '\0');
	return len;
}


void MyStrcat(char *dst, char *addendum){
	int i,j;
	i = MyStrlen(dst);
	for( j = 0; addendum[j]!='\0';j++){
		dst[i] = addendum[j];
		i++;
	}
	dst[i] = '\0';
}
int MyStrcmp(char *p, char *q, int len){
	int i,p_len,q_len;
	p_len = MyStrlen(p);
	q_len = MyStrlen(q);
	if (p_len < len || q_len < len){
		printf("Cant be compared to len");
		return 0;
	}
	for( i = 0; i <len; i++){
		if (p[i] == q[i]){
			continue;
		}else{
			return 0;
		}
	}
}
void MyStrcpy(char *dst, char *src) {
	int i,j;
	i = MyStrlen(dst)
	 char ch;
      ch = *src;
      while(ch != '\0') {
         *dst = ch;
         dst++;
         src++;
         ch = *src;
      }
      *dst = '\0';
}
void MyMemcpy(char *dst, char *src, int size){
	int i;
	for( i = 0; i <size; i++){
		dst[i] = src[i];
	}
}
