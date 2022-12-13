global inb
global inw
global indw
global outb
global outw
global outdw

inb:
mov dx, di
in al, dx
ret

inw:
mov dx, di
in ax, dx
ret

indw:
mov dx, di
in eax, dx
ret

outb:
mov dx, di
mov ax, si
out dx, al
ret

outw:
mov dx, di
mov ax, si
out dx, ax
ret

outdw:
mov dx, di
mov eax, esi
out dx, eax
ret