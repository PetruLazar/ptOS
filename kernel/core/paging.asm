[bits 64]
global setCR3
setCR3:
mov cr3, rdi
ret