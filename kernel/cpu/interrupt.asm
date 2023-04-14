[bits 64]
[extern exceptionHandler]
[extern irqHandler]
[extern os_serviceHandler]

interruptNr: db 0
errCode: dq 0

global kernelPaging
kernelPaging:
dq 0

isr_common:
call pushCpuState
; call c++ handler
lea rdi, [rsp]
movzx rsi, byte [interruptNr]
mov rdx, [errCode]
call exceptionHandler
call popCpuState
sti
iretq

irq_common:
call pushCpuState
;call c++ handler
lea rdi, [rsp]
movzx rsi, byte [interruptNr]
xor rdx, rdx
call irqHandler
call popCpuState
sti
iretq

isr_0:
cli
mov byte [interruptNr], 0x0
jmp isr_common

isr_1:
cli
mov byte [interruptNr], 0x1
jmp isr_common

isr_2:
cli
mov byte [interruptNr], 0x2
jmp isr_common

isr_3:
cli
mov byte [interruptNr], 0x3
jmp isr_common

isr_4:
cli
mov byte [interruptNr], 0x4
jmp isr_common

isr_5:
cli
mov byte [interruptNr], 0x5
jmp isr_common

isr_6:
cli
mov byte [interruptNr], 0x6
jmp isr_common

isr_7:
cli
mov byte [interruptNr], 0x7
jmp isr_common

isr_8:
cli
pop qword [errCode]
mov byte [interruptNr], 0x8
jmp isr_common

isr_9:
cli
mov byte [interruptNr], 0x9
jmp isr_common

isr_a:
cli
pop qword [errCode]
mov byte [interruptNr], 0xa
jmp isr_common

isr_b:
cli
pop qword [errCode]
mov byte [interruptNr], 0xb
jmp isr_common

isr_c:
cli
pop qword [errCode]
mov byte [interruptNr], 0xc
jmp isr_common

isr_d:
cli
pop qword [errCode]
mov byte [interruptNr], 0xd
jmp isr_common

isr_e:
cli
pop qword [errCode]
mov byte [interruptNr], 0xe
jmp isr_common

isr_f:
cli
mov byte [interruptNr], 0xf
jmp isr_common

isr_10:
cli
mov byte [interruptNr], 0x10
jmp isr_common

isr_11:
cli
pop qword [errCode]
mov byte [interruptNr], 0x11
jmp isr_common

isr_12:
cli
mov byte [interruptNr], 0x12
jmp isr_common

isr_13:
cli
mov byte [interruptNr], 0x13
jmp isr_common

isr_14:
cli
mov byte [interruptNr], 0x14
jmp isr_common

isr_15:
cli
pop qword [errCode]
mov byte [interruptNr], 0x15
jmp isr_common

isr_16:
cli
mov byte [interruptNr], 0x16
jmp isr_common

isr_17:
cli
mov byte [interruptNr], 0x17
jmp isr_common

isr_18:
cli
mov byte [interruptNr], 0x18
jmp isr_common

isr_19:
cli
mov byte [interruptNr], 0x19
jmp isr_common

isr_1a:
cli
mov byte [interruptNr], 0x1a
jmp isr_common

isr_1b:
cli
mov byte [interruptNr], 0x1b
jmp isr_common

isr_1c:
cli
mov byte [interruptNr], 0x1c
jmp isr_common

isr_1d:
cli
pop qword [errCode]
mov byte [interruptNr], 0x1d
jmp isr_common

isr_1e:
cli
pop qword [errCode]
mov byte [interruptNr], 0x1e
jmp isr_common

isr_1f:
cli
mov byte [interruptNr], 0x1f
jmp isr_common

global getRSP
getRSP:
lea rax, [rsp+8]
ret
global getRBP
getRBP:
lea rax, [rbp]
ret

isr_30:
cli
call pushCpuState
; call c++ handler
lea rdi, [rsp]
call os_serviceHandler
call popCpuState
sti
iretq

irq_0:
cli
mov byte [interruptNr], 0
jmp irq_common

irq_1:
cli
mov byte [interruptNr], 1
jmp irq_common

irq_2:
cli
mov byte [interruptNr], 2
jmp irq_common

irq_3:
cli
mov byte [interruptNr], 3
jmp irq_common

irq_4:
cli
mov byte [interruptNr], 4
jmp irq_common

irq_5:
cli
mov byte [interruptNr], 5
jmp irq_common

irq_6:
cli
mov byte [interruptNr], 6
jmp irq_common

irq_7:
cli
mov byte [interruptNr], 7
jmp irq_common

irq_8:
cli
mov byte [interruptNr], 8
jmp irq_common

irq_9:
cli
mov byte [interruptNr], 9
jmp irq_common

irq_10:
cli
mov byte [interruptNr], 10
jmp irq_common

irq_11:
cli
mov byte [interruptNr], 11
jmp irq_common

irq_12:
cli
mov byte [interruptNr], 12
jmp irq_common

irq_13:
cli
mov byte [interruptNr], 13
jmp irq_common

irq_14:
cli
mov byte [interruptNr], 14
jmp irq_common

irq_15:
cli
mov byte [interruptNr], 15
jmp irq_common

global isr_0
global isr_1
global isr_2
global isr_3
global isr_4
global isr_5
global isr_6
global isr_7
global isr_8
global isr_9
global isr_a
global isr_b
global isr_c
global isr_d
global isr_e
global isr_f
global isr_10
global isr_11
global isr_12
global isr_13
global isr_14
global isr_15
global isr_16
global isr_17
global isr_18
global isr_19
global isr_1a
global isr_1b
global isr_1c
global isr_1d
global isr_1e
global isr_1f

global isr_30

global irq_0
global irq_1
global irq_2
global irq_3
global irq_4
global irq_5
global irq_6
global irq_7
global irq_8
global irq_9
global irq_10
global irq_11
global irq_12
global irq_13
global irq_14
global irq_15

disableInterrupts:
cli
ret

enableInterrupts:
sti
ret

global disableInterrupts
global enableInterrupts

global getCR2
getCR2:
mov rax, cr2
ret

pushCpuState:
push rbp
lea rbp, [rsp + 8]
push gs
push fs
; mov bp, ds
; push rbp
push r9
push r8
push rsi
push rdi
push rdx
push rcx
push rbx
push rax
mov rax, [rbp]
mov rdi, cr3
mov [rbp], rdi
mov rdi, [kernelPaging]
mov cr3, rdi
jmp rax

popCpuState:
pop rbp ; return address
pop rax
pop rbx
pop rcx
pop rdx
pop rdi
pop rsi
pop r8
pop r9
; pop rbp
; mov ds, bp
pop fs
pop gs
xchg rbp, [rsp + 8] ; ret addr <=> stack cr3
mov cr3, rbp
pop rbp
ret