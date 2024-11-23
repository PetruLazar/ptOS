[org 0x1000]
[bits 16]

%macro DISK_READ 0
	mov ah, 0x42
	int 0x13
%endmacro

; set up stack
mov sp, 0x7c00

; cluster size at most 0x4000, checked by VBR code
; set up cluster size
mov al, byte [FAT_SECTORS_PER_CLUSTERS]
mov byte [DISK_ADDRESS_PACKET.SECTOR_COUNT], al

mov cx, 0x1000
mov fs, cx ; set up es to be the base of cluster buffer

; locate kernel on the filesystem
; kernel file is ptOS/sys/kernel.bin

; ax, bx = root dir cluster
mov ax, word [FAT_HEADER_ROOT_DIR_CLUSTER]     ; low word
mov bx, word [FAT_HEADER_ROOT_DIR_CLUSTER + 2] ; high word
; di = filename to look for, and also dir entry type
mov di, DIRNAME_PTOS
; dx = end of cluster addr
movzx dx, byte [FAT_SECTORS_PER_CLUSTERS]
shl dx, 9 ; *0x200 bytes per sector
; dx - upper limit of cluster

LOAD_CLUSTER_LOOP: ; load next cluster
call LOAD_CLUSTER
xor si, si
DIR_ENTRIES_LOOP: ; check dir entries
cmp byte [fs:si], 0 ; no more dir entries
je NO_MORE_CLUSTERS
cmp byte [fs:si], 0xe5 ; unused entry
je SKIP_DIR_ENTRY
VALID_DIR_ENTRY: ; entry used
call FILENAME_CMP ; compare current entry with target entry (filename + type)
jne SKIP_DIR_ENTRY ; not the entry needed
add di, 12 ; move to searching the next string
mov ax, word [fs:si + FAT_DIR_ENTRY.START_CLUSTER_LOW_WORD] ; start cluster - low word
mov bx, word [fs:si + FAT_DIR_ENTRY.START_CLUSTER_HIGH_WORD] ; start cluster - high word
cmp byte [fs:si + FAT_DIR_ENTRY.ATTRIBUTES], 0x04 ; entry is file entry: kernel found
je KERNEL_FOUND
jmp LOAD_CLUSTER_LOOP; next dir found, go search in it
SKIP_DIR_ENTRY:
add si, 0x20 ; dir entries are 0x20 bytes
cmp si, dx
jne DIR_ENTRIES_LOOP ; more etries in dir
DIR_ENTRY_NOT_FOUND: ; directory cluster finished with no match
; get next cluster in chain, if any
; bx:ax already set to current cluster
call GET_FAT_ENTRY
; if current cluster not the last one, jump to loading the next one
jnz LOAD_CLUSTER_LOOP

NO_MORE_CLUSTERS: ; file not found, no more clusters to load
jmp KERNEL_NOT_FOUND

KERNEL_FOUND:
; fs:si - kernel file dir entry
; bx:ax - kernel file start cluster

; check size of kernel binary
; get cluster size in bytes
mov edx, dword [fs:si + FAT_DIR_ENTRY.FILE_SIZE] ; edx - file size
movzx ecx, byte [FAT_SECTORS_PER_CLUSTERS]
shl ecx, 9 ; *= 0x200 bytes per cluster
; align size in bytes upwards to cluster size
xor edi, edi
ALIGN_UP_LOOP:
add edi, ecx ; add increments of cluster size in bytes
jc KERNEL_TOO_BIG ; if it overflows, freeze
cmp edi, edx ; while edi < edx
jb ALIGN_UP_LOOP
; edi = size of file on disk
; edi at most from 0x10000 until 0x80000 = 0x70000 bytes
cmp edi, 0x30000 ; temporarily limited to 0x30000 instead of 
				 ; 0x70000 until kernel memmap is also updated
				 ; kernel from 0x8000 to 0x38000
ja KERNEL_TOO_BIG ; force size not above limit

; get cluster size in segments
mov dx, cx
shr dx, 4 ; /= 0x10

; load kernel from the disk
; ecx = size of cluster in bytes
; dx = size of cluster in segments
; edi - size of file, aligned upwards to cluster size
; bx:ax - first cluster of file
BOOTX16_CLUSTER_CHAIN:
call LOAD_CLUSTER ; bx:ax already set
sub edi, ecx ; decrement file size left
jz BOOTX16_LOADING_FINISHED ; if no more bytes in file
add word [DISK_ADDRESS_PACKET.BUFFER_SEGMENT], dx ; increment address in DAP !!!!
call GET_FAT_ENTRY ; next cluster
jz INCONSISTENT_FS ; if no more clusters, but file bytes left
jmp BOOTX16_CLUSTER_CHAIN
BOOTX16_LOADING_FINISHED: ; no file bytes left, check cluster chain

LONG_MODE_SWITCH_START_1:
; disable interrupts
cli

; go to long mode
mov eax, 10100000b ; set "physical address extension" and "page global enabled"
mov cr4, eax

mov eax, PAGING_LOW ; point cr3 to pml4
mov cr3, eax

; enable long mode by setting EFER.LME
mov ecx, 0xC0000080
rdmsr
or eax, 0x00000100
wrmsr

; load global descriptor table
lgdt [gdt_descriptor]

; enable paging
mov eax, cr0
or eax, 0x80000001
mov cr0, eax

jmp CODE64_SEG:after_longmode_switch

LONG_MODE_SWITCH_FINISH_1:
; di, dx - destination, size
; si, cx - source, size
; assumption: dx >= cx
ADD_BIG_8_DS:
	mov cx, 8
	mov dx, cx
ADD_BIG:
	pusha
	sub dx, cx ; since dx >= cx, this also clears CF
	.MAIN_LOOP:
	lodsb
	adc al, byte [di]
	stosb
	loop .MAIN_LOOP
	pushfw ; save carry flag
	pop ax
	;setc al
	xor cx, dx ; since cx = 0, this is the same as mov cx, dx, but updates ZF
	jz .EXIT
	push ax
	popfw ; restore carry flag
	;add al, 0xff
	.PADDING_LOOP:
	adc byte [di], 0
	inc di
	loop .PADDING_LOOP
	.EXIT:
	popa
	ret

; di, dx - destination, size
; bl - multiplication factor
MUL_BIG:
	pusha
	mov cx, dx
	xor bh, bh
	.MAIN_LOOP:
	mov al, byte [di]
	mul bl
	add al, bh
	adc ah, 0
	stosb
	mov bh, ah
	loop .MAIN_LOOP
	popa
	ret

; func to get FAT entry corresponding to cluster bx:ax
; bx:ax - next cluster in chain
; if ZF set, end of cluster chain
GET_FAT_ENTRY:
	; calculate sector in which the entry is located
	; 512 / 4 = 128 FAT entries per sector
	; mov bp, sp ; create a stack frame
	; sub sp, 4 ; allocate two words on stack
	pusha
	push ax ; save for later, for sector offset
	mov cx, 7; shift bx:ax right 7 times
	.SHIFT_RIGHT:
	clc 
	rcr bx, 1 ; shift right
	rcr ax, 1
	loop .SHIFT_RIGHT
	; bx:ax = sector offset in fat

	; set up start LBA in disk address packet
	mov di, DISK_ADDRESS_PACKET_FAT_SECTOR.START_LBA
	mov word [di], ax
	mov word [di + 2], bx
	xor ax, ax
	mov word [di + 4], ax
	mov word [di + 6], ax

	; di already set up+
	mov si, FAT_OFFSET
	call ADD_BIG_8_DS

	mov si, DISK_ADDRESS_PACKET_FAT_SECTOR
	mov dl, byte [DISK_NR]
	DISK_READ
	jc DISK_READING_ERROR

	pop bx ; low word of cluter, for FAT entry offset
	and bx, 0x7f ; 128 entries per FAT, 7 bits
	shl bx, 2 ; 4 bytes per entry
	add bh, FAT_SECTOR_BUFFER >> 8 ; + 0x1000
	mov ax, word [bx]
	mov bx, word [bx + 0x2]
	and bh, 0x0f ; mask upper 4 bits
	cmp ax, 0xFFF8
	jb .EXIT ; not end of cluster chain, ZF not set
	cmp bx, 0x0FFF ; if equal, end of cluster chain, and ZF set
	; cmp 0x0FFF
	.EXIT:
	;pushfw ; save ZF for return value
	;mov [bp], ax ; save bx:ax before popa
	;mov [bp - 2], bx
	;popfw
	;popa
	;pop bx ; pop return values
	;pop ax

	; expand the popa so that the stack is restored,
	; but return value bx:ax is preserved
	pop di
	pop si
	pop bp
	pop dx
	pop dx
	pop dx
	pop cx
	; discard the old ax value on the stack,
	; but without discarding the return value in ZF
	mov bp, sp ; get sp
	lea sp, [bp + 2] ; add 2
	ret

; func to load cluster in bx:ax
LOAD_CLUSTER:
	pusha
	; sectors = cluster nr * cluster size + cluster data offset
	; init buffer
	xor bl, bl
	mov di, DISK_ADDRESS_PACKET.START_LBA
	mov dx, 8 ; 8 bytes
	call MUL_BIG

	; fill buffer and multiply
	; dx already 8 bytes
	; di already DISK_ADDRESS_PACKET.START_LBA
	mov word [di], ax ; argument ax
	mov word [di + 2], bx
	mov bl, byte [FAT_SECTORS_PER_CLUSTERS]
	call MUL_BIG

	; add cluster data offset
	; dx is already 8
	; di is already DISK_ADDRESS_PACKET.START_LBA
	mov cx, dx
	mov si, CLUSTER_OFFSET
	call ADD_BIG

	mov si, DISK_ADDRESS_PACKET
	mov dl, byte [DISK_NR]
	DISK_READ
	jc DISK_READING_ERROR
	popa
	ret

; compare two 11-byte strings
; si, di - the two strings
FILENAME_CMP:
	pusha
	push ds
	mov cx, fs
	mov ds, cx
	mov cx, 12 ;  8.3 std filename + entry type
	repe cmpsb
	pop ds
	popa
	ret

; check a20 line
check_a20: ; this only works once !!!!
	; check 0000:1000 against ffff:1010
	mov ax, 0xffff
	mov es, ax
	mov [es:0x1010], al
	mov bl, byte [0x1000]
	xor al, bl
	ret

DISK_READING_ERROR: ; failed to read from disk
	mov ax, DISK_READING_ERROR_STR
	jmp PRINT_STR
nolongmode: ; long mode not supported
	mov ax, nolongmode_STR
	jmp PRINT_STR
KERNEL_NOT_FOUND: ; failed to locate kernel
	mov ax, KERNEL_NOT_FOUND_STR
	jmp PRINT_STR
KERNEL_TOO_BIG:
	mov ax, KERNEL_TOO_BIG_STR
	jmp PRINT_STR
INCONSISTENT_FS:
	mov ax, INCONSISTENT_FS_STR
	jmp PRINT_STR

; ax - string addr
PRINT_STR:
	xor bx, bx
	mov si, ax
	mov ah, 0xe
	.STRING_LOOP:
	mov al, byte [si]
	test al, al
	jz .EXIT
	int 0x10
	inc si
	jmp .STRING_LOOP
	.EXIT:
	jmp $

; set up gdt
LONG_MODE_SWITCH_START_2:
gdt_null:
dq 0x0

gdt_code64: ; len 64 bits
	dd 0x0 ; limit low 0 - 15 and base high 0 - 15
	db 0x0 ; base 16 - 23
	db 10011010b ; access byte
	db 00100000b ; flags and limit 16 - 19
	db 0x0 ; base 24 - 31

gdt_data:
	dw 0xffff
	dw 0x0000
	db 0x00
	db 10010010b
	db 00000000b
	db 0x0

gdt_descriptor:
dw gdt_descriptor - gdt_null - 1
dq gdt_null

CODE64_SEG equ gdt_code64 - gdt_null ;  0x8
DATA_SEG equ gdt_data - gdt_null ; 0x10
LONG_MODE_SWITCH_FINISH_2:

; Disk Address Packet
DISK_ADDRESS_PACKET:
	db 0x10, 0x00
	.SECTOR_COUNT:
	dw 0x00
	.BUFFER_OFFSET:
	dw 0x0000
	.BUFFER_SEGMENT:
	dw 0x1000
	.START_LBA:
	dq 0x00

; Disk Address Packet for FAT entry
DISK_ADDRESS_PACKET_FAT_SECTOR:
	db 0x10, 0x00			; signature
	dw 0x01					; sector count
	dw FAT_SECTOR_BUFFER	; buffer offset
	dw 0x0					; buffet segment
	.START_LBA:
	dq 0x01


[bits 64]

LONG_MODE_SWITCH_START_3:
after_longmode_switch:

; load registers for long mode
mov ax, 0x10
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

mov rsp, 0x50000
push qword 0
mov rbp, rsp

LONG_MODE_SWITCH_FINISH_3:

; move paging structures to higher address
mov rsi, PAGING_LOW
mov rdi, PAGING_HIGH
mov rcx, 0x4000 / 8
cld
rep movsq
mov rax, PAGING_HIGH
mov qword [rax], PAGING_HIGH + 0x1000 + PAGE_READWRITE + PAGE_PRESENT
mov qword [rax + 0x1000], PAGING_HIGH + 0x2000 + PAGE_READWRITE + PAGE_PRESENT
mov qword [rax + 0x2000], PAGING_HIGH + 0x3000 + PAGE_READWRITE + PAGE_PRESENT
mov cr3, rax

; move kernel image to correct address
mov rsi, 0x10000
mov rdi, KERNEL_RELOCATED
mov rcx, 0x70000 / 8
rep movsq

LONG_MODE_SWITCH_START_4:
; prepare data structure for kernel to take over
push qword MEMMAP_ENTRYLIST ; pointer to mem map entries
push qword MEMMAP_ENTRYCOUNT ; pointer to mem map descriptor (count + entry size)
mov rdi, rsp ; argument for kernel's main function

; transfer control to the kernel
jmp 0x8000
LONG_MODE_SWITCH_FINISH_4:

; strings
DIRNAME_PTOS: db "PTOS       ", 0x14 ; PTOS DIR
DIRNAME_SYS: db "SYS        ", 0x14 ; SYS DIR
FILENAME_KERNEL: db "KERNEL  BIN", 0x04 ; kernel.bin FILE

DISK_READING_ERROR_STR: db "Error reading from disk", 0
nolongmode_STR: db "This machine does not support Long mode", 0
KERNEL_NOT_FOUND_STR: db "Could not locate kernel", 0
KERNEL_TOO_BIG_STR: db "Kernel file is too big", 0
INCONSISTENT_FS_STR: db "Filesystem is inconsistent/corrupted", 0

; constants
PAGE_PRESENT equ 1 << 0
PAGE_READWRITE equ 1 << 1

MEMMAP_ENTRYCOUNT equ 0x5000
MEMMAP_ENTRYSIZE equ MEMMAP_ENTRYCOUNT + 1
MEMMAP_ENTRYLIST equ 0x5010
FAT_SECTOR_BUFFER equ 0x0c00
KERNEL_RELOCATED equ 0x8000
PAGING_LOW equ 0xa000
PAGING_HIGH equ 0x100000

DISK_NR equ 0xf08 ; 1 byte
FAT_SECTORS_PER_CLUSTERS equ 0xf09 ; 1 byte, 6-byte gap
FAT_OFFSET equ 0xf10 ; 8 bytes
CLUSTER_OFFSET equ FAT_OFFSET + 0x8 ; = 0xf18, 8 bytes

FAT_HEADER_ROOT_DIR_CLUSTER equ 0x7c2c  ; 4 bytes

FAT_DIR_ENTRY:
	.FILENAME83 equ 0x00
	.ATTRIBUTES equ 0x0b
	.RESERVED_NT equ 0x0c
	.CREATION_TIME_MS equ 0x0d
	.CREATION_TIME equ 0x0e
	.CREATION_DATE equ 0x10
	.LAST_ACCESS_DATE equ 0x12
	.START_CLUSTER_HIGH_WORD equ 0x14
	.LAST_MODIFICATION_TIME equ 0x16
	.LAST_MODIFICATION_DATE equ 0x18
	.START_CLUSTER_LOW_WORD equ 0x1a
	.FILE_SIZE equ 0x1c					; 4 bytes

; times 0 - ($ - $$) nop
; times 0x1000 - ($ - $$) db 0xcc

dw (LONG_MODE_SWITCH_FINISH_1 - LONG_MODE_SWITCH_START_1) + (LONG_MODE_SWITCH_FINISH_2 - LONG_MODE_SWITCH_START_2) + (LONG_MODE_SWITCH_FINISH_3 - LONG_MODE_SWITCH_START_3) + (LONG_MODE_SWITCH_FINISH_4 - LONG_MODE_SWITCH_START_4)