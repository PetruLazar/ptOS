[extern main]

;init program heap and stuff

; call main 
call main

; give control back to os
lea rdi, [rax]
xor rbx, rbx
mov rax, 4
int 0x30