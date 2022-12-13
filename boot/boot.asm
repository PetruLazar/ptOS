[org 0x7c00]
[bits 16]

; setting up stack
jmp 0x0:bootloader_orig
bootloader_orig:
xor ax, ax
mov ss, ax
mov ds, ax
mov es, ax
mov gs, ax
mov fs, ax
mov bp, 0x8000
mov sp, bp
mov byte [bootdrive], dl

; check for a20 and enable if necessary
call check_a20
jnz a20_enabled
mov bx, 0x5000
mov word [bx], 'N'
mov cx, bx
call printstring16
jmp $

a20_enabled:

; check for cpuid

pushfd
pop eax
mov ecx, eax
xor eax, 0x200000
push eax
popfd

pushfd
pop eax
push ecx
popfd
xor eax, ecx
test eax, 0x200000
jz nolongmode

; ;check for longmode

mov eax, 0x80000000
cpuid
cmp eax, 0x80000001
jz nolongmode
mov eax, 0x80000001
cpuid
test edx, 1 << 29
jz nolongmode

;get memory map
;point es:di at 0x5000

xor ebx, ebx
mov byte [0x5000], 0
mov es, bx
mov di, 0x5000 + 24
mov edx, 0x534d4150
memmaploop:
mov eax, 0xe820
mov ecx, 24
int 0x15
jc memmapend
test ebx, ebx
jz memmapend
mov byte [0x5001], cl
add di, 24
inc byte [0x5000]
jmp memmaploop
memmapend:

; load kernel from disk
; load from disk 2 times, 0xc000 bytes (0x60 sectors) each
; 1 - 0x0800 0 to 0x1400 0, starting at sec 02
; 2 - 0x1400 0 to 0x2000 0, starting at sec 62

mov dl, [bootdrive]
xor bx, bx
mov ax, 0x800
mov es, ax
mov cx, 0x0260
xor dh, dh
clc
call diskload
jc diskload_fail

mov cx, 0x2360
mov ax, 0x1400
mov es, ax
inc dh
call diskload
jc diskload_fail

; disable interrupts
cli

; set up paging
%include "src/boot/paging.asm"

jmp 0x8000

diskload_fail:
mov cx, diskload_fail_msg
call printstring16
jmp $

nolongmode:
mov cx, nolongmode_msg
call printstring16
jmp $

check_a20:
; check 0x0000:0x7dfe (0x7dfe) against 0xffff:0x7e0e (0x107dfe)
xor ax, ax
not ax
mov es, ax
mov al, byte [0x7dfe]
mov cl, al
xor al, 1
mov byte [es:0x7e0e], al
xor cl, byte [es:0x7e0e]
ret

%include "src/boot/printstring16.asm"
%include "src/boot/diskload.asm"

bootdrive: db 0
diskload_fail_msg: db 'Disk err', 0
nolongmode_msg: db "LM err", 0

times 446 - ($ - $$) db 0

partition1:
db 0
db 2, 3, 0
db 6
db 0xfe, 0x3f, 0x81
dd 0x80
dd 0x1fe800

partitions_unused: 
dq 0, 0
dq 0, 0
dq 0, 0

dw 0xaa55