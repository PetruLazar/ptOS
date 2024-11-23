[bits 64]
[extern main]

call main
cli
hlt

[bits 64]
global makesyscall
makesyscall:
syscall
ret
