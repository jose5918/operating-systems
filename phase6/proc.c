#include "spede.h"
#include "data.h"
#include "proc.h"
#include "services.h"
#include "tools.h"


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
	int i, len, my_port;
	char login_str[BUFF_SIZE], passwd_str[BUFF_SIZE],cmd_str[BUFF_SIZE], cwd[BUFF_SIZE];
	char exit_str[] = "exit";
	char pwd_str[] = "pwd";
	char cd_str[] = "cd ";
	char ls_str[] = "ls";
	char cat_str[] = "cat ";
	
	my_port = PortAlloc(); // init port device and port_t data associated
	while (1){
		while(1){
			PortWrite("Please enter your login\n\r", my_port);
			PortWrite("Login: ", my_port);
			PortRead(login_str, my_port);
			if(MyStrlen(login_str) != 0){//NOT sure
				PortWrite("Password: ", my_port);
				PortRead(passwd_str, my_port);
				if (MyStrlen(passwd_str) == MyStrlen(login_str)){
					if (MyStrcmp(MyStrReverse(passwd_str),login_str,MyStrlen(login_str))){
						cwd[0] = '/';
						break;
					}
				}
			}
		while(1){
			PortWrite("Please enter your command\n\r", my_port);
			PortWrite("command: ", my_port);
			PortRead(cmd_str, my_port);
			if(MyStrlen(cmd_str) != 0){
				if (MyStrcmp(cmd_str,exit_str,5)){
					break;
				}
				if (MyStrcmp(cmd_str,pwd_str,4)){
					PortWrite(cwd, my_port);
				}else if (MyStrcmp(cmd_str,cd_str,3)){
					TermCd(&cmd_str[3], cwd, my_port);//[TODO]
				}else if (MyStrcmp(cmd_str,ls_str,3)){
					TermLs(cwd,my_port);
				}else if (MyStrcmp(cmd_str,cat_str,4)){
					TermCat(&cmd_str[4], cwd, my_port);//[TODO]
				}
			}
		}
	}					
}
void TermCd(char *name, char *cwd, int my_port){
	char attr_data[BUFF_SIZE];
    attr_t *attr_p;
	
	if (MyStrlen(name) == 0){
		return;
	}
	if (MyStrcmp(name,".\0",2){
		return;
	}
	if (MyStrcmp(name,"/\0",2) || MyStrcmp(name,"..\0",3)){
		cwd = '/';
		return;
	}
	attr_data = FSfind(name,cwd); //[todo]
	
	if (MyStrlen(attr_data) == 0){
		PortWrite("NOT FOUND \n\r", my_port);
		return;
	}
	
	attr_p = (attr_t *)attr_data;
	//[TODO]
	if (attr_p -> mode != D) {
		PortWrite("CANNOT CD A FILE \n\r", my_port);
		return;
	}
	MyStrcat(cwd, name);
}
void TermCat(char *name, char *cwd, int my_port){
	char read_data[BUFF_SIZE], attr_data[BUFF_SIZE];
    attr_t *attr_p;
    int my_fd;	
	//[TODO]
	attr_data = FSfind(name,cwd)
	if (MyStrlen(attr_data) == 0){
		PortWrite("NOT FOUND \n\r", my_port);
		return;
	}
	attr_p = (attr_t *)attr_data;
	//[TODO]
	if (attr_p -> mode  == 'D') {
		PortWrite("CANNOT CAT a DIRECTORY \n\r", my_port);
		return;
	}
	//[TODO]
	my_fd = FSopen(name,cwd);
	while(1){
		read_data = FSread(my_fd);//[TODO]
		if(MyStrlen(read_data)==0){
			break;
		}
		PortWrite(read_data, my_port);
	}
	FSclose(my_fd);
	
}
void TermLs(char *cwd, int my_port){
	char ls_str[BUFF_SIZE], attr_data[BUFF_SIZE];
    attr_t *attr_p;
    int my_fd;
	
	attr_data = FSfind("",cwd);
	//[TODO]
	if (attr_p -> mode != 'D') {
		PortWrite("CANNOT LS A FILE \n\r", my_port);
	}
	
	my_fd = FSopen("",cwd);
	
	while(1){
		attr_data = FSread(my_fd);
		if(MyStrlen(attr_data)==0){
			break;
		}
		attr_p = (attr_t *)attr_data;
		ls_str = Attr2str(attr_p);
		PortWrite(ls_str, my_port);
	}
	FSclose(my_fd)
}
// make str from the attributes attr_p points
   // str contains: attr_t and name (with p+1 to point to name)
void Attr2Str(attr_t *attr_p, char *str) {
      char *name = (char *)(attr_p + 1);
      sprintf(str, "     - - - -    SIZE %4d    NAME  %s\n\r", attr_p->size, name);
      if ( A_ISDIR(attr_p->mode) ) str[5] = 'D';          // mode is directory
      if ( QBIT_ON(attr_p->mode, A_ROTH) ) str[7] = 'R';  // mode is readable
      if ( QBIT_ON(attr_p->mode, A_WOTH) ) str[9] = 'W';  // mode is writable
      if ( QBIT_ON(attr_p->mode, A_XOTH) ) str[11] = 'X'; // mode is executable
}

