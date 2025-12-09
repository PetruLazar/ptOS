#include "exception.h"
#include <types.h>
#include "../../utils/isriostream.h"
#include "pic.h"
#include "../../core/scheduler.h"
#include "../../core/sys.h"

using namespace ISR::std;

namespace Exception
{
	extern "C" qword getCR2();

	const char *exceptionMessages[0x20] = {
		"Divide by 0",
		"Debug",
		"Non-maskable interrupt",
		"Breakpoint",
		"Overflow",
		"Bound range exceeded",
		"Invalid opcode",
		"Device not available",
		"Double fault",
		"Coprocessor segment overrun",
		"Invalid TSS",
		"Segment not present",
		"Stack segment fault",
		"General protection fault",
		"Page fault",
		"Reserved",
		"x87 floating-point exception",
		"Alignment check",
		"Machine check",
		"SIMD floating-point exception",
		"Virtualization exception",
		"Control protection exception",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Hypervisor injection exception",
		"VMM communication exception",
		"Security exception",
		"Reserved",
	};

	extern "C" void exceptionHandler(registers_t &regs, qword int_no, qword err_no)
	{
		// if (int_no == 7 || int_no == 6)

		cout
			<< "Exception: " << exceptionMessages[int_no] << " (0x" << std::ostream::base::hex << int_no
			<< " - err " << err_no
			<< ")\nReturn address: " << regs.cs << ':' << (void *)regs.rip
			<< "\nRegisters:"
			<< "\nrax=" << (void *)regs.rax << " rbx=" << (void *)regs.rbx << " rcx=" << (void *)regs.rcx
			<< "\nrdx=" << (void *)regs.rdx << " rdi=" << (void *)regs.rdi << " rsi=" << (void *)regs.rsi
			<< "\nrsp=" << (void *)regs.rsp << " rbp=" << (void *)regs.rbp
			<< "\nReturn address memory contents:\n";
		qword physAddress;
		if (regs.cr3->getPhysicalAddress(regs.rip, physAddress))
		{
			cout << "\tTemporarily disabled for security and stability purposes.\n";
			// isr_DisplyMemoryBlock((byte *)physAddress - 0x10, 0x20);
		}
		else
			cout << "\tInvalid virtual address.\n";

		cout << "Call stack:\n";
		cout << "\tTemporarily disabled for security and stability purposes.\n";
		// for (qword *rbp = (qword *)regs.rbp; rbp[0]; rbp = (qword *)rbp[0])
		// {
		// 	cout << "\t0x" << (void *)rbp[1] << " (file offset: 0x" << (void *)(rbp[1] - 0x8000 + 0x200) << ")\n";
		// }

		// exception specific information
		switch (int_no)
		{
		case 0xe: // page fault
			cout << "\nAccessing address: " << (void *)getCR2() << '\n';
			break;
		}

		if (!Scheduler::isEnabled() || Scheduler::getCurrentThread()->getParentTask()->isKernelTask())
		{
			cout << "kernel exception\n";
			System::blueScreen();
		}
		else
		{
			cout << "non-kernel exception\n";
			regs.rdi = (ull)-1;
			Scheduler::preempt(regs, Scheduler::preemptReason::taskExited);
		}
	}
}