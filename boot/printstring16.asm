; cx - address of null terminated string
printstring16:
push bx
push si
push ax
mov si, cx
mov ah, 0xe
xor bh, bh
printstring16_loop:
mov al, byte [si]
cmp al, 0
je printstring16_exit
int 0x10
inc si
jmp printstring16_loop
printstring16_exit:
pop ax
pop si
pop bx
ret