[bits 64]
global setCR3
setCR3:
mov cr3, rdi
ret

global getCR3
getCR3:
mov rax, cr3
ret