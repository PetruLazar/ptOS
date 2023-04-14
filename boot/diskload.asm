; es:bx - buffer pointer
; cl  - sectors to read
; ch - start sector
; dl - drive
; dh - head
; diskload:
; mov al, cl
; mov ah, 0x2
; mov cl, ch
; xor ch, ch
; and cl, 111111b
; int 0x13
; ret

; ax - sector start
; bx - sector count
; es:si - buffer
; dl - drive nr
diskload:
push word 0
push word 0
push word 0
push ax
push es
push si
push bx
push word 0x10
mov si, sp
mov ah, 0x42
int 0x13
add sp, 0x10
ret

; DAP
; +0x0: size = db 0x10
; +0x1: res = db 0
; +0x2: nrSec = dw
; +0x4: bufferSeg = dw
; +0x6: bufferOffset = dw
; +0x8: startSec = dq