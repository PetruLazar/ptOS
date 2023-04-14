global loadGDT
loadGDT:
lgdt [rdi]
ret

global getCurrentGDT
getCurrentGDT:
sgdt [rdi]
ret

global updateSegmentRegisters
updateSegmentRegisters:
mov ax, 0x10
mov ss, ax
mov ax, 0x2b
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
ret

;rdi - word gdtEntry
global loadTSS
loadTSS:
mov rax, rdi
ltr ax
ret