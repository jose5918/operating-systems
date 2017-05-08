.text
.global _start
_start  #main(), $x addr, x is content:
   mov $msg, %eax
   sub $_start, $msg
   add $0x40000000, %eax
   push %eax
   int $105
   pop %eax
   int $115

.data
msg:
   .ascii “ Hello World! Team Clang here\n\r”