; rdi - dest
; rsi - src
; rdx - len
global memmove
memmove:
mov rcx, rdx
cmp rsi, rdi
jb reverse
cld
jmp execute
reverse:
add rdi, rcx
lea rsi, [rsi+rcx-1]
dec rdi
std
execute:
rep movsb
ret
; optimize based on aligment maybe?
