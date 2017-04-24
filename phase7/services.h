#ifndef _SERVICES_H_
#define _SERVICES_H_

int GetPid();
void Sleep(int);

int SemAlloc(int);
void SemWait(int);
void SemPost(int);
void SysPrint(char *);
int PortAlloc();
void PortWrite(char *, int);
void PortRead(char *, int);
void FSfind(char *, char *, char *);
int FSopen(char *, char *);
void FSread(int, char *);
void FSclose(int);

#endif
