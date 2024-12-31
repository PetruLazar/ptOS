[org 0x100000] ; the OS will load the program at this virtual address
[bits 64]

; executable file header, recognized by the OS
dq 0x100008

; print string
mov rax, 1 ; SYSCALL_SCREEN
mov rbx, 1 ; SYSCALL_SCREEN_PRINTSTR
mov rdi, hello_string
int 0x30

; tell OS the program finished
mov rax, 4 ; SYSCALL_PROGENV
mov rbx, 0 ; SYSCALL_PROGENV_EXIT
mov rdi, 0 ; exit code
int 0x30

hello_string: db "Hello World!", 10, 0