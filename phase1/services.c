int GetPid(void){
    int my_pid;
    asm("pushl %%eax; \
         int $100; \ //service call/system call
         movl %%eax, %0
         popl %%eax"
    : "=g", (my_pid) //%0 is this thing
    :
    :);
    return my_pid;
}