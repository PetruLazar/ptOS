#pragma once
#include <types.h>
#include "../../core/paging.h"

extern "C" void disableInterrupts();
extern "C" void enableInterrupts();

struct registers_t
{
	qword rax, rbx, rcx, rdx, rdi, rsi, r8, r9,
		r10, r11, r12, r13, r14, r15,
		fs, gs, rbp;
	PageMapLevel4 *cr3;
	qword rip, cs, rflags, rsp, ss;
};

namespace IDT
{
	void Initialize(byte *IDT_address);
};
