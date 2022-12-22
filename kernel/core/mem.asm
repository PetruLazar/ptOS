; rdi - dest
; rsi - src
; rdx - len
global memcpy
memcpy:
mov rcx, rdx
rep movsb
ret
; optimize using alignment maybe?
; ; test if src and dest have the same aligment
; mov rcx, rdx
; mov rax, rdi
; xor rax, rsi
; test rax, 7
; jnz jmp1
; ; same alignment
; and rcx, 7
; sub rdx, rcx
; rep movsq
; jmp1:
;
; mov rcx, rdx
; rep movsb
; ret


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
; optimize this one too using aligment maybe? and also the next one?

; extern "C" void memset(void *ptr, ull len, byte val)
; rdi - ptr
; rsi - len
; dl - val
global memset
memset:
cld
lea rcx, [rsi]
mov al, dl
rep stosb
ret