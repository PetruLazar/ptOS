#pragma once
#include <types.h>
#include "../core/paging.h"

constexpr int KERNEL_CS = 0x8;

extern "C" void disableInterrupts();
extern "C" void enableInterrupts();

struct registers_t
{
	qword rax, rbx, rcx, rdx, rdi, rsi, r8, r9,
		fs, gs, rbp;
	PageMapLevel4 *cr3;
	qword rip, cs, rflags, rsp, ss;
};

typedef void (*voidf)();
typedef void (*IrqHandler)(registers_t &);

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

		void setInterruptGate(voidf offset, byte ist);
		void setTrapGate(voidf offset, byte ist);
	};

	static void Initialize();

	static void registerIrqHandler(byte irq_no, IrqHandler handler);
	static void waitForIrq1();
};
