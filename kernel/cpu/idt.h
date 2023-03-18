#pragma once
#include "../../libc/types.h"
#include "../core/paging.h"

constexpr int KERNEL_CS = 0x8;

typedef void (*voidf)();

extern "C" void disableInterrupts();
extern "C" void enableInterrupts();

struct registers_t
{
	qword rax, rbx, rcx, rdx, rdi, rsi, r8, r9,
		fs, gs, rbp;
	PageMapLevel4 *cr3;
	qword rip, cs, rflags, rsp, ss;
};

class IDT
{
public:
	class Gate
	{
	public:
		word offsetLow = 0;
		word segmentSelector = 0;
		byte IST = 0;	// bits 3-7 are reserved
		byte flags = 0; // 0-3 gate type (def to interrupt gate), 4=0,5-6 dpl, 7 - present
		word offsetMid = 0;
		dword offsetHigh = 0;
		dword reserved = 0;

		void setInterruptGate(voidf offset);
		void setTrapGate(voidf offset);
	};

	static void Initialize();

	static void registerIrqHandler(byte irq_no, voidf handler);
	static void waitForIrq1();
};
