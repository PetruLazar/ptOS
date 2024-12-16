[bits 64]
[default rel]

[section .text]

global idleTask
idleTask:
hlt
mov rax, 1
mov rax, 2
lea rdi, [afterhltmsg]
here:
jmp here

[section .data]
afterhltmsg:
db "After hlt", 10, 0