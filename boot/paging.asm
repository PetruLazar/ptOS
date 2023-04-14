; free 16kb: 0x1000 to 0x4fff
xor ax, ax
mov es, ax
mov di, 0x1000
mov ecx, 0x1000
xor eax, eax
cld
rep stosd
mov di, 0x1000

PAGE_PRESENT equ 1 << 0
PAGE_READWRITE equ 1 << 1

; build pages
; pml4
lea eax, [es:di + 0x1000] ; address of the page directory pointer table into eax
or eax, PAGE_PRESENT | PAGE_READWRITE ; set flags
mov [es:di], eax ; store the value as the first entry

; pdpt
lea eax, [es:di + 0x2000] ; address of the page directory into eax
or eax, PAGE_PRESENT | PAGE_READWRITE ; set flags
mov [es:di + 0x1000], eax ; store the value as the first entry 

; pd
lea eax, [es:di + 0x3000] ; adress of the page table into eax
or eax, PAGE_PRESENT | PAGE_READWRITE ; set flags
mov [es:di + 0x2000], eax ; store the value as the first entry

; pt
lea di, [di + 0x3000] ; point to the pt
mov eax, PAGE_PRESENT | PAGE_READWRITE ; set flags for memory block 0x0000
pagetable_loop:
mov [es:di], eax ; place memory block at [eax] in page table entry
add eax, 0x1000 ; advance to the next 4kb memory block
add di, 8 ; advance one entry in the page table
cmp eax, 0x200000 ; check if not all 2mb have been mapped
jb pagetable_loop ; if not, continue to loop