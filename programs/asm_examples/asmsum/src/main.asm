[org 0x100000] ; the OS will load the program at this virtual address
[bits 64]

; executable file header, recognized by the OS
dq entry

entry:
; prompt user for first number
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 1 ; SYSCALL_SCREEN_PRINTSTR
mov rdi, prompt_1 ; get string address
int 0x30

; read single digit number
mov rax, 2 ; SYSCALL_KEYBOARD
mov rbx, 4 ; SYSCALL_KEYBOARD_KEYPRESSEDEVENT_CHAR
mov rdi, 1 ; blocking call
int 0x30
; OS returned char in rax

mov r8, rax ; save first number
mov rdi, rax ; prepare for echo
sub r8, '0' ; get actual number read, instead of the ASCII value

; echo
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
; rdi already set
int 0x30

; newline after first number
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
mov rdi, 10 ; newline character
int 0x30

; prompt user for second number
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 1 ; SYSCALL_SCREEN_PRINTSTR
mov rdi, prompt_2 ; get string address
int 0x30

; read second single digit number
mov rax, 2
mov rbx, 4
mov rdi, 1
int 0x30
; OS returned char in rax

add r8, rax ; add the numbers together
mov rdi, rax ; prepare for echo

; echo
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
; rdi already set
int 0x30

; newline after second number
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
mov rdi, 10 ; newline character
int 0x30

; display message before result
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 1 ; SYSCALL_SCREEN_PRINTSTR
mov rdi, msg_result ; get string address
int 0x30

; output the result
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
mov rdi, r8 ; r8 contains the sum
int 0x30

; newline after result
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 2 ; SYSCALL_SCREEN_PRINTCH
mov rdi, 10 ; newline character
int 0x30

; tell OS the program finished
mov rax, 4 ; SYSCALL_PROGENV
mov rbx, 0 ; SYSCALL_PROGENV_EXIT
mov rdi, 0 ; exit code
int 0x30

prompt_1: db "a = ", 0
prompt_2: db "b = ", 0
msg_result: db "a + b = ", 0