[org 0x8000]
[bits 16]

INIT:
	; disable interrupts
	cli
	; replace handlers
	xor ax, ax
	mov ds, ax

	mov di, 0xc ; handler for int 3
	mov word [di], INT3_HANDLER
	mov word [di + 2], 0

	mov di, 0x4 ; handler for int 1
	mov word [di], INT1_HANDLER
	mov word [di + 2], 0

	; debugger debugging preconditions here
	

	; enable interrupts
	sti

	; turn on single stepping
	pushfw
	push bp
	mov bp, sp
	xor word [bp + 2], 1 << TRAP_FLAG_BIT; set trap flag
	pop bp
	popfw ; apply flags register, this will take effect after the next instruction is finished

	; debugger debug code here
	

	ret

INTERRUPT_STACK equ 0x1a
STACK_FL equ 0x18
STACK_CS equ 0x16
STACK_IP equ 0x14
STACK_INTN equ 0x12
STACK_AX equ 0x10
STACK_BX equ 0x0e
STACK_CX equ 0x0c
STACK_DX equ 0x0a
STACK_DI equ 0x08
STACK_SI equ 0x06
STACK_DS equ 0x04
STACK_ES equ 0x02
STACK_BP equ 0x00

TRAP_FLAG_BIT equ 0x8

INT1_HANDLER:
	push word 1
	jmp INT_HANDLER

INT3_HANDLER:
	push word 3
	jmp INT_HANDLER

; handler for int 1 and int3
; stack contains:
; pushf
; push cs
; push ip
INT_HANDLER:
	; save registers
	cld
	push ax
	push bx
	push cx
	push dx
	push di
	push si
	push ds
	push es
	push bp
	mov bp, sp

	; init ds
	xor ax, ax
	mov ds, ax

	; adjust interrupt return address
	mov ax, word [bp + STACK_INTN]
	shr ax, 1
	sub word [bp + STACK_IP], ax
	
	; restore all breakpoints if breakpoints are currently applied
	cmp word [bp + STACK_INTN], 3
	jne .NO_BP_RESTORE
	call RESTORE_ALL
	.NO_BP_RESTORE:

	; if stepping but not user stepping
	;	disable stepping, user stepping, reapply breakpoints and exit interrupt
	mov al, byte [FLAGS]
	xor al, FLAGS.STEPPING
	test al, FLAGS.STEPPING | FLAGS.USER_STEPPING
	jnz .SKIP_EXIT_CLAUSE_1
	mov byte [FLAGS], 0
	and word [bp + STACK_FL], ~(1 << TRAP_FLAG_BIT)
	call REAPPLY_ALL
	jmp .INTERRUPT_EXIT
	.SKIP_EXIT_CLAUSE_1:

	; if stepping until return
	; 	if current instruction is not a ret instruction
	;		silently exit the interrupt
	;	else (current instruction is ret instruction)
	;		clear stepping until return flag
	test byte [FLAGS], FLAGS.STEPPING_TIL_RETURN
	jz .SKIP_EXIT_CLAUSE_2 ; stepping until return bit is not set

	mov ax, [bp + STACK_CS] ; get current cs
	mov es, ax
	mov bx, [bp + STACK_IP] ; get current ip
	mov al, byte [es:bx] ; get current instruction
	or al, 0x01 ; last bit is not important;
	cmp al, 0xc3
	je .AT_RETURN_INSTR ; this is a ret instruction (opcode C2 or C3)
	cmp al, 0xcb
	je .AT_RETURN_INSTR ; this is a retf instruction (opcode CA or CB)
	; this is not a return instruction
	jmp .INTERRUPT_EXIT
	.AT_RETURN_INSTR:
	; this is a ret or retf instruction
	xor byte [FLAGS], FLAGS.STEPPING_TIL_RETURN
	.SKIP_EXIT_CLAUSE_2:

	; store old cursor 
	mov ah, 0x03
	xor bx, bx
	int 0x10 ; get current cursor por
	mov word [CURSOR_POS], dx ; save current pos

	; save screen contents
	mov ax, 0xb800
	mov ds, ax
	mov ax, SCREEN_BUFFER / 0x10
	mov es, ax
	xor ax, ax
	mov si, ax ; 0
	mov di, ax ; 0
	mov cx, 80 * 25 ; one screen of words
	rep movsw


	.RETRY_INPUT:
	; clear screen
	mov ax, 0xb800
	mov es, ax
	mov ax, 0x0720 ; white spaces on black background
	xor di, di
	mov cx, 80 * 25
	rep stosw

	; reset cursor
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ah, 0x2
	xor bx, bx
	mov dx, bx
	int 0x10 ; reset cursor

	; reset cmd buffer
	mov byte [CMD_BUFFER_POS], 0

	; display registers
	call DISPLAY_ALL_REGS

	; read cmd
	mov cx, 4 ; 4 char cmd
	.CMD_READ_LOOP:
	call GET_LETTER
	call PRINT_CHAR
	movzx di, byte [CMD_BUFFER_POS]
	mov byte [CMD_BUFFER + di], al
	inc byte [CMD_BUFFER_POS]
	loop .CMD_READ_LOOP

	; evaluate command using cmd table
	mov bx, CMD_COUNT * 4
	.EVALUATION_LOOP:
	mov di, CMD_BUFFER
	mov si, word [CMD_TABLE + bx - 4 + CMD_TABLE_ENTRY_STROFFSET]
	mov cx, 4 ; 4 char string
	mov ax, .WRONG_STRING
	repe cmpsb ; compare strings
	cmove ax, word [CMD_TABLE + bx - 4 + CMD_TABLE_ENTRY_JMPOFFSET]
	jmp ax ; if strings are equal, jump to offset in CMD_TABLE
	.WRONG_STRING:
	sub bx, 4 ; next cmd in CMD_TABLE
	jnz .EVALUATION_LOOP
	jmp CASE_INVALID ; this executes if cmd not found in CMD_TABLE

	.INTERRUPT_EXIT:
	; restore screen
	mov ax, 0xb800
	mov es, ax
	mov ax, SCREEN_BUFFER / 0x10
	mov ds, ax
	xor ax, ax
	mov si, ax ; 0
	mov di, ax ; 0
	mov cx, 80 * 25 ; one screen of words
	rep movsw

	; restore cursor
	xor ax, ax
	mov ds, ax
	mov ah, 0x2
	xor bx, bx
	mov dx, word [CURSOR_POS]
	int 0x10

	; restore registers
	pop bp
	pop es
	pop ds
	pop si
	pop di
	pop dx
	pop cx
	pop bx
	pop ax

	add sp, 2

	; return from interrupt
	iret

REAPPLY_ALL:
	movzx cx, byte [BREAKPOINT_COUNT]
	test cx, cx
	jz .EXIT
	.LOOP:
	mov bx, cx
	shl bx, 3 ; * 8 -- 8 bytes per breakpoint
	mov es, word [BREAKPOINT_LIST + bx + BREAKPOINT_SEGMENT - 8] ; get bp seg
	mov di, word [BREAKPOINT_LIST + bx + BREAKPOINT_OFFSET - 8] ; get bp off
	mov dl, byte [es:di] ; save current instruction
	mov byte [BREAKPOINT_LIST + bx + BREAKPOINT_INSTRUCTION - 8], dl
	mov byte [es:di], 0xcc ; place the int3 instruction
	loop .LOOP
	.EXIT:
	ret

RESTORE_ALL:
	movzx cx, byte [BREAKPOINT_COUNT]
	test cx, cx
	jz .EXIT
	.LOOP:
	mov bx, cx
	shl bx, 3 ; * 8 -- 8 bytes per breakpoint
	mov es, word [BREAKPOINT_LIST + bx + BREAKPOINT_SEGMENT - 8] ; get bp seg
	mov di, word [BREAKPOINT_LIST + bx + BREAKPOINT_OFFSET - 8] ; get bp offs
	mov dl, byte [BREAKPOINT_LIST + bx + BREAKPOINT_INSTRUCTION - 8] ; get replaced instruction
	mov byte [es:di], dl ; place original instruction
	loop .LOOP
	.EXIT:
	ret

CASE_LIST: ; list - list breakpoints
	; print an  endl
	mov ax, ENDL_STR
	call PRINT_STR

	movzx bx, byte [BREAKPOINT_COUNT]
	test bx, bx
	jz .NO_BP
	shl bx, 3 ; * 8 -- 8 bytes per breakpoint
	.MAIN_LOOP:
	mov dx, word [BREAKPOINT_LIST + bx - 8 + BREAKPOINT_OFFSET]
	mov cx, word [BREAKPOINT_LIST + bx - 8 + BREAKPOINT_SEGMENT]
	call PRINT_FULL_ADDRESS
	mov al, ' '
	call PRINT_CHAR
	sub bx, 8
	jnz .MAIN_LOOP

	; exit
	call GET_LETTER
	jmp INT_HANDLER.RETRY_INPUT

	.NO_BP:
	mov ax, .NO_BP_STR
	call PRINT_STR
	call GET_LETTER
	jmp INT_HANDLER.RETRY_INPUT
	.NO_BP_STR: db "No BP set", 0

CASE_STBP: ; stbp <addr> - toggle bp
	; print a space
	mov al, ' '
	call PRINT_CHAR

	; read full addr
	call GET_FULL_ADDRESS
	; cx:dx - user address

	; iterate through breakpoints
	movzx si, byte [BREAKPOINT_COUNT]
	test si, si
	jz .BP_NOT_FOUND
	shl si, 3 ; * 8 -- 8 bytes per breakpoint
	.MAIN_LOOP:
	mov ax, word [BREAKPOINT_LIST + si - 8 + BREAKPOINT_OFFSET]
	mov bx, word [BREAKPOINT_LIST + si - 8 + BREAKPOINT_SEGMENT]
	call COMPARE_ADDR
	; if bp found, remove it
	je CASE_STBP_REMOVE
	sub si, 8
	jnz .MAIN_LOOP

	.BP_NOT_FOUND:
	; if bp not found and not at 64 BPs yet, add it
	movzx si, byte [BREAKPOINT_COUNT]
	cmp si, 64 ; check that 
	; less than 64 BPs
	jb CASE_STBP_CREATE
	; maybe list breakpoints?
	; too many BPs
	jmp INT_HANDLER.RETRY_INPUT

; cx:dx - breakpoint location
CASE_STBP_CREATE:
	push bx
	movzx bx, byte [BREAKPOINT_COUNT]
	shl bx, 3 ; * 8 -- 8 bytes per breakpoint
	mov word [BREAKPOINT_LIST + bx + BREAKPOINT_OFFSET], dx ; put segmet offset in the breakpoint list
	mov word [BREAKPOINT_LIST + bx + BREAKPOINT_SEGMENT], cx ; put segment in the breakpoint list
	inc byte [BREAKPOINT_COUNT]
	pop bx
	jmp INT_HANDLER.RETRY_INPUT

; si - bp index after the one to be removed
CASE_STBP_REMOVE:
	; copy all bp's to the right one over to the left
	; calculate how many bytes to move
	movzx bx, byte [BREAKPOINT_COUNT]
	shl bx, 3 ; * 8 -- 8 bytes per BP
	mov cx, bx ; breakpoint count - deleted index + 1 = breakpoint move count
	sub cx, si

	; set up destination and source
	add si, BREAKPOINT_LIST
	lea di, [si - 8]
	xor ax, ax
	mov ds, ax
	mov es, ax

	; copy all BPs after the deleted one over to the left
	rep movsb

	; decrement BREAKPOINT_COUNT
	dec byte [BREAKPOINT_COUNT]
	jmp INT_HANDLER.RETRY_INPUT

; bx:ax - first addr
; cx:dx - second add
; ret: zero-flag set if equal
COMPARE_ADDR:
	push ax
	push bx
	push cx
	push dx
	; inc bx while ax > 0x10 and bx != 0xffff
	.LOOP_1:
	cmp ax, 0x10
	jb .LOOP_2
	cmp bx, 0xffff
	je .LOOP_2
	sub ax, 0x10
	inc bx
	jmp .LOOP_1

	.LOOP_2:
	cmp dx, 0x10
	jb .SIMPLIFIED
	cmp cx, 0xffff
	je .SIMPLIFIED
	sub dx, 0x10
	inc cx
	jmp .LOOP_2

	.SIMPLIFIED:
	xor ax, dx
	jne .EXIT
	xor bx, cx
	.EXIT:
	pop dx
	pop cx
	pop bx
	pop ax
	ret

CASE_DSCR: ; briefly display the screen
	; display screen
	push ds
	mov ax, 0xb800
	mov es, ax
	mov ax, SCREEN_BUFFER / 0x10
	mov ds, ax
	xor ax, ax
	mov si, ax ; 0
	mov di, ax ; 0
	mov cx, 80 * 25 ; one screen of words
	rep movsw

	; restore cursor
	xor ax, ax
	mov ds, ax
	mov ah, 0x2
	xor bx, bx
	mov dx, word [CURSOR_POS]
	int 0x10

	; wait for keyboard input
	call GET_LETTER

	; go back to the interrupt handler
	pop ds
	jmp INT_HANDLER.RETRY_INPUT

CASE_GETS: ; gets <addr> <len> - get byte string
	; get byte string at address and length
	; read command arguments
	mov al, ' '
	call PRINT_CHAR ; print a space
	call GET_FULL_ADDRESS ; read address
	push cx
	push dx
	mov al, ' '
	call PRINT_CHAR ; print a space
	mov cx, 2
	call GET_HEXSTRING ; read length
	mov si, dx ; length
	pop di ; address
	pop es
	; if 0x10 aligned, skip, since it will be evaluated again in the main loop
	test di, 0xf
	jz .MAIN_LOOP
	call CASE_GETS_NEWLINE
	.MAIN_LOOP:
	; if 0x10 aligned, newline + address
	test di, 0xf
	jnz .SKIP_NEWLINE
	call CASE_GETS_NEWLINE
	.SKIP_NEWLINE:
	mov al, ' '
	call PRINT_CHAR
	mov cx, 2
	mov dh, byte [es:di]
	call PRINT_HEXSTRING
	inc di ; increment address
	dec si ; decrement length
	test si, 0xff
	jnz .MAIN_LOOP
	call GET_LETTER ; wait for keystroke
	jmp INT_HANDLER.RETRY_INPUT
; di - address
CASE_GETS_NEWLINE:
	mov ax, ENDL_STR
	call PRINT_STR
	mov dx, di
	mov cx, es
	call PRINT_FULL_ADDRESS
	mov al, ':'
	call PRINT_CHAR
	ret

CASE_SETS: ; sets <addr> <len> <val> - sets bytes in string to value
	; set bytes in byte string at address and length to value
	mov ax, TEST_STR_SETS
	call PRINT_STR
	jmp INT_HANDLER.RETRY_INPUT
	jmp $
DISPLAY_ALL_REGS:
	; display the value of ax, bx, cx, dx, di, si, ds, es
	mov bx, REGISTERS
	mov si, STACK_AX
	.FIRST_LOOP:
	mov dx, word [bp + si]
	call DISPLAY_REG
	add bx, 5
	sub si, 2
	cmp si, STACK_ES
	jae .FIRST_LOOP

	; display sp
	mov bx, REG_SP
	lea dx, word [bp + INTERRUPT_STACK]
	call DISPLAY_REG

	; display cs:ip
	mov bx, REG_CS_IP
	mov dx, word [bp + STACK_CS]
	call DISPLAY_REG
	mov al, ':'
	call PRINT_CHAR
	mov dx, word [bp + STACK_IP]
	mov cx, 4
	call PRINT_HEXSTRING

	; display flags
	mov bx, REG_FL
	mov dx, word [bp + STACK_FL]
	call DISPLAY_REG

	; display 8 bytes of program
	mov ax, PROG_BYTES
	call PRINT_STR
	push ds ; save previous ds
	mov si, word [bp + STACK_IP] ; get interrupted ip
	mov bx, word [bp + STACK_CS] ; get interrupted cs
	mov ds, bx ; use cs to retrieve data
	mov bx, 8 ; 8 bytes
	call DISPLAY_BLOCK

	; display 8 bytes of stack
	mov ax, STACK_BYTES
	pop ds
	call PRINT_STR ; ds-dependent, pop ds and push again
	push ds
	lea si, word [bp + INTERRUPT_STACK] ; get interrupted sp
	mov bx, ss ; get ss (same as interrupted)
	mov ds, bx ; use ss to retrieve data
	mov bx, 8
	call DISPLAY_BLOCK
	pop ds

	; endl and return
	mov ax, ENDL_STR
	call PRINT_STR
	ret

; bx - counter
; si - start address
DISPLAY_BLOCK:
	mov al, ' ' ; space
	call PRINT_CHAR
	mov cx, 2 ; two digits
	mov dh, byte [si] ; block byte
	call PRINT_HEXSTRING
	inc si ; next byte
	dec bx
	jnz DISPLAY_BLOCK
	ret

; bx - string, dx - value
DISPLAY_REG:
	mov ax, bx
	call PRINT_STR
	mov al, ' '
	call PRINT_CHAR
	mov cx, 4
	call PRINT_HEXSTRING
	ret

REGISTERS:
db "  AX", 0
db "  BX", 0
db "  CX", 0
db "  DX", 0
db "  DI", 0
db "  SI", 0
db "  DS", 0
db "  ES", 0

REG_SP: db 13, 10, "  SP", 0
REG_CS_IP: db "  CS:IP", 0
REG_FL: db "  FLAGS", 0
PROG_BYTES: db 13, 10, "PROG:", 0
STACK_BYTES: db "  STACK:", 0
ENDL_STR: db 13, 10, 0

CASE_CONT: ; cont - resume execution
	; disable user stepping and enable stepping
	; resume execution
	mov byte [FLAGS], FLAGS.STEPPING
	or word [bp + STACK_FL], 1 << TRAP_FLAG_BIT
	jmp INT_HANDLER.INTERRUPT_EXIT

CASE_STEP: ; step - execute current instruction and stop
	; enable stepping and user stepping
	; resume execution
	mov byte [FLAGS], FLAGS.STEPPING | FLAGS.USER_STEPPING
	or word [bp + STACK_FL], 1 << TRAP_FLAG_BIT
	jmp INT_HANDLER.INTERRUPT_EXIT

CASE_SOVR:
	; display 'not imeplented' string
	mov ax, TEST_STR_SOVR
	call PRINT_STR

	; wait for keystroke and exit
	call GET_LETTER
	jmp INT_HANDLER.RETRY_INPUT

CASE_SOUT:
	; display 'not imeplented' string
	mov ax, TEST_STR_SOUT
	call PRINT_STR

	; wait for keystroke and exit
	call GET_LETTER
	jmp INT_HANDLER.RETRY_INPUT

CASE_SRET: ; sret - step until return
	; enable stepping, user stepping and stepping
	mov byte [FLAGS], FLAGS.STEPPING | FLAGS.USER_STEPPING | FLAGS.STEPPING_TIL_RETURN
	jmp INT_HANDLER.INTERRUPT_EXIT

CASE_INVALID:
	; display "invalid command"
	; go to read command again
	mov ax, TEST_STR_INVALID
	call PRINT_STR
	jmp INT_HANDLER.RETRY_INPUT

; ret al - char
GET_LETTER:
	push dx
	.READ_LOOP:
	xor ah, ah
	int 0x16
	mov dl, al
	sub dl, 'a' ; no less than a
	cmp dl, 'z' - 'a' ; no more than z
	ja .READ_LOOP
	pop dx
	ret

; ret al - character
; ret ah - value
GET_HEX:
	.READ_LOOP:
	xor ah, ah
	int 0x16
	mov ah, al
	cmp ah, '9'
	ja .OVER_10
	.UNDER_10:
	sub ah, '0'
	cmp ah, '9' - '0'
	ja .READ_LOOP
	jmp .EXIT
	.OVER_10:
	sub ah, 'a'
	cmp ah, 'f' - 'a'
	ja .READ_LOOP
	add ah, 10
	.EXIT:
	ret

; al - value
PRINT_HEX:
	push bx
	push cx
	mov cx, 'a' - 10
	mov bx, '0'
	cmp al, 9
	cmova bx, cx
	add al, bl
	call PRINT_CHAR
	pop cx
	pop bx
	ret

; al - character
PRINT_CHAR:
	push ax
	push bx
	mov ah, 0xe
	xor bx, bx
	int 0x10
	pop bx
	pop ax
	ret

; ax - string addr
PRINT_STR:
	push si
	mov si, ax
	.STRING_LOOP:
	mov al, byte [si]
	test al, al
	jz .EXIT
	call PRINT_CHAR
	inc si
	jmp .STRING_LOOP
	.EXIT:
	pop si
	ret

; cx - digit count (2 or 4)
; ret dx - result
GET_HEXSTRING:
	xor dx, dx
	.MAIN_LOOP:
	call GET_HEX
	shl dx, 4
	or dl, ah
	call PRINT_CHAR ; echo character
	loop .MAIN_LOOP
	ret

; ret cx:dx - address
GET_FULL_ADDRESS:
	mov cx, 4
	push cx
	call GET_HEXSTRING ; read segment
	pop cx
	push dx
	mov al, ':'
	call PRINT_CHAR
	call GET_HEXSTRING
	pop cx
	ret

; cx:dx - address
PRINT_FULL_ADDRESS:
	push dx
	mov dx, cx
	mov cx, 4
	push cx
	call PRINT_HEXSTRING
	mov al, ':'
	call PRINT_CHAR
	pop cx
	pop dx
	call PRINT_HEXSTRING
	ret

; cx - digit count (2 or 4)
; for cx = 2: dh - value
; for cx = 4: dx - value
PRINT_HEXSTRING:
	.MAIN_LOOP:
	mov al, dh
	shr al, 4
	call PRINT_HEX
	shl dx, 4
	loop .MAIN_LOOP
	ret

; constants
CMD_COUNT equ 10
CMD_TABLE_ENTRY_STROFFSET equ 0x0
CMD_TABLE_ENTRY_JMPOFFSET equ 0x2
CMD_TABLE:

dw CMD_LIST
dw CASE_LIST

dw CMD_STBP
dw CASE_STBP

dw CMD_DSCR
dw CASE_DSCR

dw CMD_GETS
dw CASE_GETS

dw CMD_SETS
dw CASE_SETS

dw CMD_CONT
dw CASE_CONT

dw CMD_STEP
dw CASE_STEP

dw CMD_SOVR
dw CASE_SOVR

dw CMD_SOUT
dw CASE_SOUT

dw CMD_SRET
dw CASE_SRET

CMD_LIST: db "list"
CMD_STBP: db "stbp"
CMD_DSCR: db "dscr"
CMD_GETS: db "gets"
CMD_SETS: db "sets"
CMD_CONT: db "cont"
CMD_STEP: db "step"
CMD_SOVR: db "sovr"
CMD_SOUT: db "sout"
CMD_SRET: db "sret"

TEST_STR_SETS: db 13, 10, "SETS not implemented yet", 13, 10, 0
TEST_STR_SOVR: db 13, 10, "SOVR not implemented yet", 13, 10, 0
TEST_STR_SOUT: db 13, 10, "SOUT not implemented yet", 13, 10, 0
TEST_STR_INVALID: db 13, 10, "INVALID not implemented yet", 13, 10, 0

; initialized vars
BREAKPOINT_COUNT: db 0 ; at most 64 BPs
FLAGS: db 0x00
	.STEPPING equ 0x1				; bit 0
	.USER_STEPPING equ 0x2			; bit 1
	.STEPPING_TIL_RETURN equ 0x10	; bit 4
	.STEPPING_OUT equ 0x20			; bit 5
	.STEPPING_OVER equ 0x40			; bit 6

; uninitialized vars
; each breakpoint in the breakpoint list has the 4 byte address for the segment and segment offset,
; and the 1 byte that was replaced, padded to 4 bytea, total 8 bytes
; maximum 64 breakpoints
BREAKPOINT_LIST equ 0x8800 ; 64 entries * 8 bytes = 0x200 bytes
	BREAKPOINT_OFFSET equ 0x0
	BREAKPOINT_SEGMENT equ 0x2
	BREAKPOINT_INSTRUCTION equ 0x4
CMD_BUFFER 		equ BREAKPOINT_LIST + 0x200 ; 4 bytes
CMD_BUFFER_POS 	equ CMD_BUFFER + 0x4 ; 1 byte, aligned to 2 bytes
CURSOR_POS 		equ CMD_BUFFER_POS + 0x2 ; 2 byte
SCREEN_BUFFER 	equ 0x9000 ; 80 * 25 * 2 = 0xfa0 (rounded to 0x1000 bytes)

; sector padding
times 512 * 3 - ($ - $$) int3

; jump to BIOS HANDLER: