; es:bx - buffer pointer
; cl  - sectors to read
; ch - start sector
; dl - drive
; dh - head
diskload:
mov al, cl
mov ah, 0x2
mov cl, ch
xor ch, ch
and cl, 111111b
int 0x13
ret