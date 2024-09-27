[org 0x7c00]
[bits 16]

; magic values
	VALID_SIGNATURE_1 equ 0x28
	VALID_SIGNATURE_2 equ 0x29

%macro DISK_READ 0
	mov ah, 0x42
	int 0x13
%endmacro

VBR_STUB:
	jmp VBR_BOOTCODE
	times 3 - ($ - $$) nop
FAT_HEADER:
	.oemId: dq 0
	.bytes_per_sector: dw 0x200
	.sectors_per_cluster: db 0x08
	.reserved_sectors: dw 0x0006
	.FAT_count: db 0x02
	.rootdir_entry_count: dw 0x2 
	.volume_sector_count: dw 0x0
	.media_descriptor_type: db 0x0
	.sectors_per_FAT: dw 0xffff
	.sectors_per_track: dw 0x0
	.nr_of_heads: dw 0x0
	.hidden_sector_count: dd 0x00000008
	.volume_large_sector_count: dd 0x0
FAT32_HEADER:
	.sectors_per_FAT: dd 0x00000001
	.flags: dw 0x0
	.FAT_version: dw 0x0
	.root_dir_cluster: dd 0x2
	.FSInfo_sector: dw 0x1
	.backup_vbr_sector: dw 0x3
	.reserved: db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	.drive_nr: db 0x0
	.flags_NT: db 0x0
	.signature: db 0x28
	.volume_ID: dd 0x0
	.volume_label: db "ptOS FAT32 "
	.ID_string: db "FAT32   "

VBR_BOOTCODE:
	mov ax, 0xb800
	mov es, ax ; es = 0xb800
	xor ax, ax
	mov di, ax ; di = 0x0
	mov ds, ax
	cld

	; clear screen
	mov ax, (00000111b << 8) | ' ' ; = 0x0720 ; fill with spaces, black background, light gray foreground
	mov cx, 80 * 25
	rep stosw
	mov es, cx

	; set up registers
	; dl already set to disk index
	mov byte [DISK_NR], dl

	; do some partition cheks
	; check partition signature
	mov al, byte [FAT32_HEADER.signature]
	or al, 0x1 ; force bit 0 to a 1, so 0x28 becomes 0x29
	cmp al, 0x29 ; compare with 0x29
	jnz INVALID_SIGNATURE_ERROR ; invalid signature

	; prepare loading second sector
	mov di, DISK_ADDRESS_PACKET.START_LBA
	mov dx, 8
	mov si, FAT_HEADER.hidden_sector_count
	mov cl, 4
	call ADD_BIG ; +1 to hidden sector count
	mov dl, byte [DISK_NR]

	; load second sector
	mov si, DISK_ADDRESS_PACKET
	DISK_READ
	jc DISK_READING_ERROR
	mov byte [DISK_ADDRESS_PACKET.BUFFER_OFFSET + 1], 0x10 ; 7E 00 -> 10 00

	; check if cluster is at most 0x4000, aka sectors per cluster at most 0x20
	movzx ax, byte [FAT_HEADER.sectors_per_cluster]
	cmp al, 0x20 ; = 0x4000 bytes / 0x200 byte-sectors
	ja CLUSTER_TOO_BIG ; bytes in cluster more than 0x4000
	mov byte [FAT_SECTORS_PER_CLUSTERS], al
	mov word [DISK_ADDRESS_PACKET.SECTOR_COUNT], ax
	shl ax, 1
	push ax ; save 2 * sectors_per_cluster for later

	; init some partition-specific values
	xor bx, bx
	mov di, FAT_OFFSET
	mov dx, 8 + 8 ; length of FAT_OFFSET + CLUSTER_OFFSET
	call MUL_BIG
	
	; FAT_OFFSET = hidden_sectors + reserved_sectors
	mov ax, word [FAT_HEADER.reserved_sectors]
	; di already FAT_OFFSET
	mov word [di], ax
	; cx is already 4
	mov si, FAT_HEADER.hidden_sector_count
	shr dx, 1 ; 16 /= 2 = 8
	call ADD_BIG ; 

	; CLUSTER_OFFSET = FAT_OFFSET + (FAT_count * sectors_per_FAT) - (2 * sectors_per_cluster)
	; calculate (FAT_count * sectors_per_FAT)
	mov di, CLUSTER_OFFSET
	; dx is already 8
	mov si, FAT32_HEADER.sectors_per_FAT
	; cx already 4
	call ADD_BIG ; CLUSTER_OFFSET = 0, equ to an assignment

	; di, dx already set
	mov bl, byte [FAT_HEADER.FAT_count]
	call MUL_BIG
	; finished calculating (FAT_count * sectors_per_FAT)

	; calculate FAT_OFFSET + (FAT_count * sectors_per_FAT)
	; di, dx already set
	mov si, FAT_OFFSET
	call ADD_BIG_8_DS
	; finished calculating FAT_OFFSET + (FAT_count * sectors_per_FAT)

	; calculate FAT_OFFSET + (FAT_count * sectors_per_FAT) - (2 * sectors_per_cluster)
	mov al, 0xFF ; previous MUL_BIG and ADD_BIG leave ax clear
	; cx already 8
	mov di, SECTOR_MULT_BUFFER ; temporary buffer
	push di ; save SECTOR_MULT_BUFFER for later
	pusha
	rep stosb
	popa

	pop si ; SECTOR_MULT_BUFFER
	; cx already 8
	mov di, CLUSTER_OFFSET

	pop ax ; 2 * sectorsPerCluster
	neg ax
	mov byte [si], al
	call ADD_BIG_8_DS
	; finished calculating FAT_OFFSET + (FAT_count * sectors_per_FAT) - (2 * sectors_per_cluster)

	; jump to second sector of VBR
	jmp SECOND_SECTOR

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
	mov bl, byte [FAT_HEADER.sectors_per_cluster]
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

DISK_READING_ERROR:
	mov ax, DISK_READING_ERROR_STR
	jmp PRINT_STR

INVALID_SIGNATURE_ERROR:
	mov ax, INVALID_SIGNATURE_ERROR_STR
	jmp PRINT_STR

CLUSTER_TOO_BIG:
	mov ax, CLUSTER_TOO_BIG_STR
	jmp PRINT_STR

BOOT16_NOT_FOUND:
	mov ax, BOOT16_NOT_FOUND_STR
	jmp PRINT_STR

BOOT16_INCOMPATIBLE:
	mov ax, BOOT16_INCOMPATIBLE_STR
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

; initialized variables and strings
DIRNAME_PTOS: db "PTOS       ", 0x14 ; PTOS DIR
DIRNAME_SYS: db "SYS        ", 0x14 ; SYS DIR
FILENAME_BOOT: db "BOOTX16 BIN", 0x04 ; BOOTX16.bin FILE
DISK_READING_ERROR_STR: db "DISK ERR", 0
INVALID_SIGNATURE_ERROR_STR: db "INVAL PART", 0
CLUSTER_TOO_BIG_STR: db "CLUSTER TOO BIG", 0
BOOT16_NOT_FOUND_STR: db "NO BL", 0
BOOT16_INCOMPATIBLE_STR: db "BL INCOMPAT", 0

; Disk Address Packet
DISK_ADDRESS_PACKET:
	db 0x10, 0x00
	.SECTOR_COUNT:
	dw 0x01
	.BUFFER_OFFSET:
	dw 0x7e00
	.BUFFER_SEGMENT:
	dw 0x0
	.START_LBA:
	dq 0x01

; uninitialized variables
SECTOR_MULT_BUFFER equ 0xf00 ; 8 bytes
DISK_NR equ 0xf08 ; 1 byte
FAT_SECTORS_PER_CLUSTERS equ 0xf09 ; 1 byte, 6-byte gap
FAT_OFFSET equ 0xf10 ; 8 bytes
CLUSTER_OFFSET equ FAT_OFFSET + 0x8 ; = 0xf18, 8 bytes

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

; vbr padding
times 510 - ($ - $$) int3

; boot signature
dw 0xaa55

; second sector
SECOND_SECTOR:
	; look for the ptOS/sys/bootx16.bin file

	; set file to look for and root cluster
	; read cluster previously set
	; look through all dir entries
	;	if entry found
	;		if directory, get cluster and jump back to reading cluster
	;		if file, then bootX16 has been found; load and jump to it
	; entry not found, read next cluster
	;	if next cluster exists, jump back to reading cluster
	;	if next cluster does not exist
			; bootX16 not found; display error and freeze

	; ax, bx = root dir cluster
	mov ax, word [FAT32_HEADER.root_dir_cluster]     ; low word
	mov bx, word [FAT32_HEADER.root_dir_cluster + 2] ; high word
	; di = filename to look for, and also dir entry type
	mov di, DIRNAME_PTOS
	; dx = end of cluster addr
	movzx dx, byte [FAT_HEADER.sectors_per_cluster]  
	shl dx, 9 ; *0x200 bytes per sector
	add dh, 0x10 ; dx += 0x1000 ; cluster starts at 0x1000
	; dx = upper limit of cluster

	.LOAD_CLUSTER_LOOP: ; load next cluster
	call LOAD_CLUSTER
	mov si, 0x1000
	.DIR_ENTRIES_LOOP: ; check dir entries
	cmp byte [si], 0 ; no more dir entries
	je .NO_MORE_CLUSTERS
	cmp byte [si], 0xe5 ; unused entry
	je .SKIP_DIR_ENTRY
	.VALID_DIR_ENTRY: ; entry used
	call FILENAME_CMP ; compare current entry with target entry (filename + type)
	jne .SKIP_DIR_ENTRY ; not the entry needed
	add di, 12 ; move to searching the next string
	mov ax, word [si + FAT_DIR_ENTRY.START_CLUSTER_LOW_WORD] ; start cluster - low word
	mov bx, word [si + FAT_DIR_ENTRY.START_CLUSTER_HIGH_WORD] ; start cluster - high word
	cmp byte [si + FAT_DIR_ENTRY.ATTRIBUTES], 0x04 ; entry is file entry: bootX16 found
	je .BOOTX16_FOUND
	jmp .LOAD_CLUSTER_LOOP; next dir found, go search in it
	.SKIP_DIR_ENTRY:
	add si, 0x20 ; dir entries are 0x20 bytes
	cmp si, dx
	jne .DIR_ENTRIES_LOOP ; more etries in dir
	.DIR_ENTRY_NOT_FOUND: ; directory cluster finished with no match
	; get next cluster in chain, if any
	; bx:ax already set to current cluster
	call GET_FAT_ENTRY
	; if current cluster not the last one, jump to loading the next one
	jnz .LOAD_CLUSTER_LOOP

	.NO_MORE_CLUSTERS: ; file not found, no more clusters to load
	jmp BOOT16_NOT_FOUND

	.BOOTX16_FOUND:
	; si - bootX16 file dir entry
	; bx:ax = bootX16 file start cluster
	
	; check file size to make sure it fits in memory
	; get cluster size in bytes
	cmp word [si + FAT_DIR_ENTRY.FILE_SIZE + 2], 0 ; high word
	jne BOOT16_INCOMPATIBLE ; must be 0
	mov dx, word [si + FAT_DIR_ENTRY.FILE_SIZE] ; dx - file size
	movzx cx, byte [FAT_HEADER.sectors_per_cluster]
	shl cx, 9 ; *= 0x200 bytes per cluster
	; align size in bytes upwards to cluster size
	xor di, di
	.ALIGN_UP_LOOP:
	add di, cx ; add increments of cluster size in bytes
	jc BOOT16_INCOMPATIBLE ; if it overflows, freeze
	cmp di, dx ; while di < dx
	jb .ALIGN_UP_LOOP
	; di = size of file on disk
	; di at most from 0x1000 until 0x7000 = 0x6000 bytes
	cmp di, 0x6000
	ja BOOT16_INCOMPATIBLE ; force size not above limit

	; cx already cluster size
	; DAP buffer already 0x1000

	; load cluster chain
	; cx = size of cluster in bytes
	; di - size of file, aligned upwards to cluster size
	; bx:ax - first cluster of file
	.BOOTX16_CLUSTER_CHAIN:
 	call LOAD_CLUSTER ; bx:ax already set

	sub di, cx ; decrement file size left
	jz .BOOTX16_LOADING_FINISHED ; if no more bytes in file
	add word [DISK_ADDRESS_PACKET.BUFFER_OFFSET], cx 	; increment address in DAP
	call GET_FAT_ENTRY ; next cluster
	jz INCONSISTENT_FS ; if no more clusters, but file bytes left
	jmp .BOOTX16_CLUSTER_CHAIN
	.BOOTX16_LOADING_FINISHED: ; no file bytes left, check cluster chain

	; sanity checks
	call GET_FAT_ENTRY
	jnz INCONSISTENT_FS ; clusters left in chain, an error occured

	; jump to boot16 code
	jmp 0x0:0x1000


; compare two 11-byte strings
; si, di - the two strings
FILENAME_CMP:
	pusha
	mov cx, 12 ;  8.3 std filename + entry type
	repe cmpsb
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

INCONSISTENT_FS:
	mov ax, INCONSISTENT_FS_STR
	jmp PRINT_STR

FAT_SECTOR_BUFFER equ 0x0c00

INCONSISTENT_FS_STR: db "INCONSISTENT FS", 0
DISK_ADDRESS_PACKET_FAT_SECTOR:
	db 0x10, 0x00			; signature
	dw 0x01					; sector count
	dw FAT_SECTOR_BUFFER	; buffer offset
	dw 0x0					; buffet segment
	.START_LBA:
	dq 0x01

times 512 * 2 - ($ - $$) int3
;times 512 - ($ - $$) int3