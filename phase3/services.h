#ifndef _SERVICES_H_
#define _SERVICES_H_

int GetPid();      
void Sleep(int);   
int SemAlloc(int);
void SemPost(int);
void SemWait(int);
#endif
