#include "spede.h"
#include "data.h"
#include "proc.h"
#include "services.h"


void Init(void) {
  int i;
  while(1){
    for (i = 0; i < LOOP; i++) {
      asm("inb $0x80");
    }
  }

}


void UserProc(void) {
  while (1) {
    cons_printf("%d..", GetPid());
  Sleep(GetPid());
  }
}

void Vehicle(void) {
   int i, my_pid;

   if(vehicle_sid == -1) vehicle_sid = SemAlloc(3);
   my_pid = GetPid();

   while(1) {
      ch_p[my_pid*80+45] = 0xf00 + 'f';
      for(i=0; i<LOOP; i++) asm("inb $0x80");
      SemWait(vehicle_sid);
      ch_p[my_pid*80+45] = 0xf00 + 'o';
      Sleep(1);
      SemPost(vehicle_sid);
   }
}

void TermProc(void) {
  int my_port;
  char str_read[BUFF_SIZE]; // size 101
  my_port = PortAlloc(); // init port device and port_t data associated
  while(1) {
      PortWrite("Hello, World! Team Clang here!\n\r", my_port);
      PortWrite("Now enter: ", my_port);
      PortRead(str_read, my_port);
      cons_printf("Read from port #%d: %s\n", my_port, str_read);
   }
}

