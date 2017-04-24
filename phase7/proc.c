#include "spede.h"
#include "data.h"
#include "proc.h"
#include "services.h"
#include "tools.h"


void Init(void) {
  int i;
  char key;

  while (1) {
    if (cons_kbhit()) {
      key = cons_getchar();
      switch (key) {
        case 'b':
          breakpoint();
      }
    }
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
	int len,my_port,i,exit_num;
	char login_str[BUFF_SIZE], passwd_str[BUFF_SIZE],cmd_str[BUFF_SIZE], cwd[BUFF_SIZE], reverse_passwd_str[BUFF_SIZE], exit_num_str[BUFF_SIZE];
	char exit_str[] = "exit\0";
	char pwd_str[] = "pwd\0";
	char cd_str[] = "cd ";
	char ls_str[] = "ls\0";
	char cat_str[] = "cat ";
	char echo_str[] = "echo\0";
	exit_num = 0;

	my_port = PortAlloc(); // init port device and port_t data associated
	while (1){
		while(1){
			PortWrite("Please enter your login\n\r", my_port);
			PortWrite("Team Clang Login: ", my_port);
			PortRead(login_str, my_port);
      len = MyStrlen(login_str);
			if(len != 0){//NOT sure
				PortWrite("Team Clang Password: ", my_port);
				PortRead(passwd_str, my_port);
        // get reverse of password
        for (i =0 ; i < len; i++){
          reverse_passwd_str[i] = passwd_str[(len-1)-i];
        }
				if (MyStrlen(passwd_str) == len){
					if (MyStrcmp(reverse_passwd_str,login_str,len)){
            MyBzero((char *)&cwd[0],sizeof(char)*BUFF_SIZE);
						cwd[0] = '/';
						exit_num = 0;
						break;
					}
				}
			}
    }
		while(1){
			PortWrite("Please enter your command\n\r", my_port);
			PortWrite("Team Clang command: ", my_port);
			PortRead(cmd_str, my_port);
			if(MyStrlen(cmd_str) != 0){

				if (MyStrcmp(cmd_str,exit_str,5)){
					break;
				}
				if (MyStrcmp(cmd_str,pwd_str,4)){
					PortWrite(cwd, my_port);
          PortWrite("\n\r",my_port);
				}else if (MyStrcmp(cmd_str,cd_str,3)){
					TermCd(&cmd_str[3], cwd, my_port);
				}else if (MyStrcmp(cmd_str,ls_str,3)){
					TermLs(cwd,my_port);
				}else if (MyStrcmp(cmd_str,cat_str,4)){
					TermCat(&cmd_str[4], cwd, my_port);
				}else if (MyStrcmp(cmd_str, echo_str, 5)){
					sprintf(exit_num_str, "%d\n\r", exit_num);
					PortWrite(exit_num_str, my_port);
				}else{
					exit_num = TermBin(cmd_str, cwd, my_port);
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
	if (MyStrcmp(name,".\0",2)){
		return;
	}
	if (MyStrcmp(name,"/\0",2) || MyStrcmp(name,"..\0",3)){
    MyBzero((char *)&cwd[0],sizeof(char)*BUFF_SIZE);
    cwd[0] = '/';
		return;
	}
	FSfind(name,cwd,attr_data);
	
	if (MyStrlen(attr_data) == 0){
		PortWrite("NOT FOUND \n\r", my_port);
		return;
	}
	
	attr_p = (attr_t *)attr_data;
	if (attr_p -> mode != MODE_DIR) {
		PortWrite("CANNOT CD A FILE \n\r", my_port);
		return;
	}
	MyStrcat(cwd, name);
  MyStrcat(cwd, "/");
}
void TermCat(char *name, char *cwd, int my_port){
	char read_data[BUFF_SIZE], attr_data[BUFF_SIZE];
    attr_t *attr_p;
    int my_fd;	
	FSfind(name,cwd,attr_data);
	if (MyStrlen(attr_data) == 0){
		PortWrite("NOT FOUND \n\r", my_port);
		return;
	}
	attr_p = (attr_t *)attr_data;
	if (attr_p -> mode  == MODE_DIR) {
		PortWrite("CANNOT CAT a DIRECTORY \n\r", my_port);
		return;
	}
	my_fd = FSopen(name,cwd);
	while(1){
		FSread(my_fd, read_data);
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
	
	FSfind("",cwd,attr_data);
  attr_p = (attr_t *) attr_data;
	if (attr_p -> mode != MODE_DIR) {
		PortWrite("CANNOT LS A FILE \n\r", my_port);
	}
	
	my_fd = FSopen("",cwd);
	
	while(1){
		FSread(my_fd, attr_data);
		if(MyStrlen(attr_data)==0){
			break;
		}
		attr_p = (attr_t *)attr_data;
		Attr2Str(attr_p, ls_str);
		PortWrite(ls_str, my_port);
	}
	FSclose(my_fd);
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

int TermBin(char *name, char *cwd, int my_port){
	char childpid_str[BUFF_SIZE], attr_data[BUFF_SIZE];
  attr_t *attr_p;
	FSfind(name,cwd,attr_data);
	if (MyStrlen(attr_data) == 0){
		PortWrite("NOT FOUND \n\r", my_port);
		return 1;
	}
	attr_p = (attr_t *)attr_data;
	if (attr_p -> mode  != MODE_EXEC) {
		PortWrite("NOT AN EXECUTABLE \n\r", my_port);
		return 1;
	}
	sprintf(childpid_str, "%d \n\r", Fork(attr_p->data));
	PortWrite(childpid_str, my_port);
	return Wait();
}
