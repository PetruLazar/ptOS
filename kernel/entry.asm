[bits 64]
[extern main]

call main
cli
hlt

global printnr64
global printstring64
global testint

testint:
mov rdi, 15
lea si, [50]
call printnr64
int 10h
;mov rdi, 16
mov si, 50 + 80
call printnr64
ret

; rdi - unsigned number 
; si - print position
printnr64:
push rbp
mov rbp, rsp
lea rax, [rdi]
dec rsp
mov byte [rsp], 0
printnr64_loop:
xor rdx, rdx
mov rbx, 10
div rbx
mov cl, dl
add cl, '0'
dec rsp
mov byte [rsp], cl
test rax, rax
jnz printnr64_loop
mov rdi, rsp
call printstring64
mov rsp, rbp
pop rbp
ret

; rdi - address of string
; si - print position
printstring64:
and rsi, 0xffff
lea rbx, [0xb8000 + rsi * 2]
printstring64_loop:
mov al, byte [rdi]
test al, al
jz printstring64_end
mov byte [rbx], al
lea rbx, [rbx + 2]
inc rdi
jmp printstring64_loop
printstring64_end:
ret

[bits 32]
; edi - address of string
; si - print position
dd 0
printstring32:
and esi, 0xffff
lea ebx, [0xb8000 + esi * 2]
printstring32_loop:
mov al, byte [edi]
test al, al
jz printstring32_end
mov byte [ebx], al
lea ebx, [ebx + 2]
inc edi
jmp printstring32_loop
printstring32_end:
; mov eax, 0x11111111
; push eax
; shl eax, 1
; push eax
; shl eax, 1
; push eax
; shl eax, 1
; push eax
; mov eax, 0x11111111
; push eax
; shl eax, 1
; push eax
; shl eax, 1
; push eax
; shl eax, 1
; push eax
mov byte [0xfff], 0x27
int 30h
; add esp, 4*8
jmp returnTo64Bits

[bits 64]
global makesyscall
makesyscall:
syscall
ret

[bits 32]
global returnTo64Bits
returnTo64Bits:
sub esp, 8
mov dword [esp], returnTo64Bits_afterTrans
mov dword [esp+4], 0x8
; push word 0x8
; push dword returnTo64Bits_afterTrans
call dword far [esp]
[bits 64]
returnTo64Bits_afterTrans:
;take the value above the 4 dwords pushed by the far call to 64 bit mode, the return address of the far call to 32 bit mode
mov rax, [rsp+4*4]
;clear the stack: 4 dwords pushed by the far call to 64 bit mode, and 4 qwords pushed by the far call to 32 bit mode
;add rsp, 4*8+4*4
jmp rax

global testCompMode
testCompMode:
push rbp
mov rbp, rsp
mov r8d, dword [0x4ff7c]
mov rax, 0xffffffff
and r8, rax
mov rax, 0x2b2b2b2b00000000
or r8, rax
lea rdi, [testCompModeStr]
mov rsi, 80*24
push qword 0x18
push qword printstring32
call qword far [rsp]
mov rsp, rbp
pop rbp
ret

testCompModeStr: db "Hello world from 32 bits mode!", 0