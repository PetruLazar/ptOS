global idleTask
idleTask:
hlt
mov rax, 1
mov rax, 2
lea rdi, [rel afterhltmsg]
here:
jmp here
afterhltmsg:
db "After hlt", 10, 0