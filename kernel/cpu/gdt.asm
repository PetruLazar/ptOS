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
mov ds, ax
mov ss, ax
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