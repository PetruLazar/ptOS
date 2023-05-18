global clock

clock:
rdtsc
and eax, 0xffffffff
shl rdx, 32
or rax, rdx
ret