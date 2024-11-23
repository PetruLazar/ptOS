[org 0x1000]
[bits 16]

%macro DISK_READ 0
	mov ah, 0x42
	int 0x13
%endmacro

; set up registers
xor ax, ax
mov ds, ax
mov ss, ax

; relocate code
mov si, LOADED_CODE_LOCATION ; current location
mov di, CODE_RELOCATION_TARGET ; relocation target
cld ; go forwards while copying
mov cx, 0x100 ; 256 words (sector is 512 bytes)
rep movsw ; copy the bootsector

; jump to reloated code
jmp 0x0:bootloader_orig
bootloader_orig:

; initialize stack
mov sp, MBR_STACK

; check a20 line - sets es, has to come before es is initialized with correct value
call check_a20 ; this function only works once !!!!
jz A_20_ERR ; A20 is disabled, halt

; clear screen
mov ax, 0xb800
mov es, ax ; es = 0xb800
xor di, di
cld

mov ax, (00000111b << 8) | ' ' ; = 0x0720 ; fill with spaces, black background, light gray foreground
mov cx, 80 * 25
rep stosw
mov es, cx


; check for cpuid: flip bit 21 (0x200000) of eflags
pushfd ; bp + 4 bytes

pushfd ; bp bytes
mov bp, sp
xor word [bp + 2], 0x20
popfd

pushfd
mov ax, word [bp + 6]
xor ax, word [bp + 2]
popfd

popfd
test al, 0x20
jz NO_LM_ERR ; no cpuid

push dx ; save boot disk nr
; check long mode compatibility
mov edi, 0x80000000
mov eax, edi
inc edi
cpuid
cmp eax, edi
jz NO_LM_ERR
mov eax, edi
cpuid
test edx, 1 << 29
jz NO_LM_ERR

pop dx ; restore boot disk nr

MBR_LOOKUP:
	mov si, DISK_ADDRESS_PACKET
	; look for bootable partitions in MBR partition table
	xor bx, bx ; start with partition entry 0
	.main_loop:
	mov cx, word [partition_table + bx + PARTITION_LBA_START + 2] ; high word
	mov di, word [partition_table + bx + PARTITION_LBA_START] 	  ; low word
	; if type is EE, the disk is GPT partitioned
	mov al, byte [partition_table + bx + PARTITION_TYPE]
	cmp al, 0xEE ; GPT partitioning detected
	je GPT_LOOKUP
	; if status & 0x80, boot
	cmp al, 0x0c ; FAT32 with LBA
	jne .continue_loop
	mov al, byte [partition_table + bx + PARTITION_STATUS]
	test al, 0x80
	jnz LOAD_VBR
	.continue_loop:
	add bx, PARTITION_ENTRY_SIZE
	cmp bx, 4 * PARTITION_ENTRY_SIZE
	jne .main_loop ; until partition index is 4
	jmp $

GPT_LOOKUP:
	GPT_HEADER equ 0x1200
		GPT_PARTITION_ARRAY_LBA equ GPT_HEADER + 0x48	; 8 bytes
		GPT_PARTITION_COUNT equ GPT_HEADER + 0x50		; 4 bytes
		GPT_PARTITION_ENTRY_SIZE equ GPT_HEADER + 0x54	; 4 bytes
	GPT_PARTITION_ARRAY equ 0x8000
		GPT_PARTITION_TYPE_GUID equ 0x0 	; 16 bytes
		GPT_PARTITION_GUID equ 0x10 		; 16 bytes
		GPT_PARTITION_START_LBA equ 0x20 	; 8 bytes
		GPT_PARTITION_END_LBA equ 0x28 		; 8 bytes
		GPT_PARTITION_ATTRIBUTES equ 0x30 	; 8 bytes
		GPT_PARTITION_NAME equ 0x38 		; variable size
	; look for bootable partitions in GPT partition table
	; read GPT header
	mov word [DISK_ADDRESS_PACKET.START_LBA + 2], cx	; high word
	mov word [DISK_ADDRESS_PACKET.START_LBA], di		; low word
	DISK_READ ; values already set up in DISK_ADDRESS_PACKET
	jc DISK_ERR ; check for read error
	mov ax, word [GPT_PARTITION_COUNT] ; partition count
	mov bx, word [GPT_PARTITION_ENTRY_SIZE] ; partition entry size, in bytes
	push ax ; save a copy of partition count
	push bx ; save a copy of partition size
	push dx ; dl = disk number
	mul bx ; multiply partition entry size with partition count to get size of partition array
	test ax, 0x1ff ; is ax aligned to 0x200 boundary?
	jz .value_aligned
	stc ; not aligned, set carry
.value_aligned:
	mov bx, 0x200
	div bx ; divide by 512 to get the number of sectors
	adc ax, 0 ; add carry to ax, if ax was not aligned to 0x200 boundary
	mov word [DISK_ADDRESS_PACKET.SECTOR_COUNT], ax
	pop dx ; pop disk number
	; set-up buffer
	mov word [DISK_ADDRESS_PACKET.BUFFER_SEGMENT], GPT_PARTITION_ARRAY >> 4
	mov word [DISK_ADDRESS_PACKET.BUFFER_OFFSET], 0x00
	mov cx, 4 ; 4 words
	push si ; save DISK_ADDRESS_PACKET
	mov si, GPT_PARTITION_ARRAY_LBA
	mov di, DISK_ADDRESS_PACKET.START_LBA
	rep movsw ; copy start LBA from GPT header to the DAP
	pop si ; restore DISK_ADDRESS_PACKET
	DISK_READ
	jc DISK_ERR ; inf loop if error
	; loop through the partitions
	pop bx ; size of partition entry
	; word [sp] - number of partitions
	mov bp, sp ; bp is used for accessing the number of partitions directly from memory
	mov ax, GPT_PARTITION_ARRAY >> 4
	mov ds, ax ; ds = segment of the partition array
	mov di, cx ; cx is already 0
.main_loop:
	; check partition entry, load VBR if bootable
	; check partition type GUID; if 0, unused entry
	push si ; save DISK_ADDRESS_PACKET
	push cx ; save partition index
	mov cx, 16 ; 16 bytes
	xor ax, ax
	lea si, [di + GPT_PARTITION_TYPE_GUID]
.type_check_loop:
	lodsb ; get byte in al
	or ah, al ; or current byte with byte buffer in ah
	loop .type_check_loop
	pop cx ; pop partition index
	pop si ; restore DISK_ADDRESS_PACKET
	test ax, ax ; check if OR byte buffer is 0
	jz .continue_loop ; ax is 0 only if all bytes of partition type are 0
	; check attributes; if bit 2 is set, boot
	mov ax, [di + GPT_PARTITION_ATTRIBUTES]	; get word 0 of attributes
	test ax, 100b ; test bit 2
	jnz .partition_found

.continue_loop:
	add di, bx ; increment partition entry pointer
	inc cx ; increment partition entry index
	cmp cx, [ss:bp]
	jne .main_loop ; loop back if not finished

	jmp NO_VALID_PART ; finished, no partition found

.partition_found:
	mov cx, word [di + GPT_PARTITION_START_LBA + 2]  ; high word
	mov di, word [di + GPT_PARTITION_START_LBA] 	 ; low word
	xor ax, ax ; reset ds
	mov ds, ax
	; jumping into LOAD_VBR by itself, since it is the next portion

; load volume boot record
LOAD_VBR:
; load LBA cx:di
; mov word [DISK_ADDRESS_PACKET.SECTOR_COUNT], 0x5 ; 2 sector ; +3 sectors for debugging
mov word [DISK_ADDRESS_PACKET.SECTOR_COUNT], 0x1 ; 2 sector ; +3 sectors for debugging
mov word [DISK_ADDRESS_PACKET.BUFFER_SEGMENT], 0x0 ; segment
mov word [DISK_ADDRESS_PACKET.BUFFER_OFFSET], 0x7c00 ; segment offset
mov word [DISK_ADDRESS_PACKET.START_LBA + 2], cx ; high word LBA
mov word [DISK_ADDRESS_PACKET.START_LBA], di ; low word LBA
DISK_READ
jc DISK_ERR
; if debugging, uncomment the next two lines
; push 0x7c00
; jmp 0x0:0x7c00 + 0x400
jmp 0x0:0x7c00

; bp - string
PRINT_STR:
	mov ax, 0x0
	mov es, ax ; string segment
	movzx cx, byte [bp] ; set nr of chars
	inc bp ; string offset
	mov bx, 0x07 ; page number + color
	mov ax, 0x1300 ; set write mode and function
	mov dx, (11 << 8) + 0
	int 0x10
	jmp $

check_a20: ; this only works once !!!!
	; check 0000:1000 against ffff:1010
	mov ax, 0xffff
	mov es, ax
	mov [es:0x1010], al
	mov bl, byte [0x1000]
	xor al, bl
	ret

DISK_ERR_STR: db 8, "DISK ERR"
NO_VALID_PART_STR: db 7, "NO PART"
NO_LM_ERR_STR: db 5, "NO LM"
A_20_ERR_STR: db 6, "NO A20"

DISK_ERR:
	mov bp, DISK_ERR_STR
	jmp PRINT_STR

NO_VALID_PART:
	mov bp, NO_VALID_PART_STR
	jmp PRINT_STR

NO_LM_ERR:
	mov bp, NO_LM_ERR_STR
	jmp PRINT_STR

A_20_ERR:
	mov bp, A_20_ERR_STR
	jmp PRINT_STR

DISK_ADDRESS_PACKET:
	db 0x10, 0x00
	.SECTOR_COUNT:
	dw 0x01
	.BUFFER_OFFSET:
	dw GPT_HEADER
	.BUFFER_SEGMENT:
	dw 0x0
	.START_LBA:
	dq 0x01

; bootloader padding
times 446 - ($ - $$) int3

; MBR partition table will be put in later

MBR_STACK equ 0x7c00
LOADED_CODE_LOCATION equ 0x7c00
CODE_RELOCATION_TARGET equ 0x1000
PARTITION_ENTRY_SIZE equ 0x10 ; size of an MBR partition table entry
; offset for the fields of a partition table entry
PARTITION_STATUS equ 0x0
PARTITION_FIRST_CHS equ 0x1
PARTITION_TYPE equ 0x4
PARTITION_LAST_CHS equ 0x5
PARTITION_LBA_START equ 0x8
PARTITION_LBA_LEN equ 0xc

; first partition entry
partition_table:
partition1:
db 0x80
db 2, 3, 0
db 6
db 0xfe, 0x3f, 0x81
dd 0x80
dd 0x1fe800

; 3 unused partitino entries
partitions_unused: 
dq 0, 0
dq 0, 0
dq 0, 0

; boot signature
dw 0xaa55