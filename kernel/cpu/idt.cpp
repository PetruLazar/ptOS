#include "idt.h"
#include "../utils/isriostream.h"
#include "pic.h"
#include "../core/sys.h"
#include "../core/paging.h"
#include "../core/scheduler.h"
#include "gdt.h"

using namespace ISR::std;

extern PageMapLevel4 *kernelPaging;

namespace IDT
{
	namespace IntEntryPoints
	{
		// exceptions
		extern "C" void isr_0();
		extern "C" void isr_1();
		extern "C" void isr_2();
		extern "C" void isr_3();
		extern "C" void isr_4();
		extern "C" void isr_5();
		extern "C" void isr_6();
		extern "C" void isr_7();
		extern "C" void isr_8();
		extern "C" void isr_9();
		extern "C" void isr_a();
		extern "C" void isr_b();
		extern "C" void isr_c();
		extern "C" void isr_d();
		extern "C" void isr_e();
		extern "C" void isr_f();
		extern "C" void isr_10();
		extern "C" void isr_11();
		extern "C" void isr_12();
		extern "C" void isr_13();
		extern "C" void isr_14();
		extern "C" void isr_15();
		extern "C" void isr_16();
		extern "C" void isr_17();
		extern "C" void isr_18();
		extern "C" void isr_19();
		extern "C" void isr_1a();
		extern "C" void isr_1b();
		extern "C" void isr_1c();
		extern "C" void isr_1d();
		extern "C" void isr_1e();
		extern "C" void isr_1f();

		// irqs
		extern "C" void irq_0();
		extern "C" void irq_1();
		extern "C" void irq_2();
		extern "C" void irq_3();
		extern "C" void irq_4();
		extern "C" void irq_5();
		extern "C" void irq_6();
		extern "C" void irq_7();
		extern "C" void irq_8();
		extern "C" void irq_9();
		extern "C" void irq_10();
		extern "C" void irq_11();
		extern "C" void irq_12();
		extern "C" void irq_13();
		extern "C" void irq_14();
		extern "C" void irq_15();

		// syscall interrupt
		extern "C" void isr_30();
	}

	extern "C" void loadidt(void *);
	extern "C" void clearint();

	constexpr int IDT_LENGTH = 256;

	class IDT_descriptor
	{
		word size;
		word offset[4];

	public:
		inline IDT_descriptor() : size(0) { setOffset(0); }
		inline IDT_descriptor(qword offset, word size) : size(size) { setOffset(offset); }

		inline void setSize(word size_)
		{
			size = size_;
		}
		inline void setOffset(qword offset_)
		{
			*(qword *)offset = offset_;
		}

		inline word getSize()
		{
			return size;
		}
		inline qword getOffset()
		{
			return *(qword *)offset;
		}
	};
	Gate *gates;
	std::vector<IrqHandler> *irqHandlers;
	// IrqHandler irqHandlers[16];
	byte irqOffset;

	void Gate::setInterruptGate(voidf funOffset, byte ist, byte dpl)
	{
		qword offset = (qword)funOffset;
		offsetLow = offset & 0xffff;
		offsetMid = (offset >> 16) & 0xffff;
		offsetHigh = offset >> 32;
		segmentSelector = GDT::KERNEL_CS;
		flags = 0b10001110 | (dpl << 5);
		IST = ist;
	}
	void Gate::setTrapGate(voidf funOffset, byte ist, byte dpl)
	{
		qword offset = (qword)funOffset;
		offsetLow = offset & 0xffff;
		offsetMid = (offset >> 16) & 0xffff;
		offsetHigh = offset >> 32;
		segmentSelector = GDT::KERNEL_CS;
		flags |= 0b10001111 | (dpl << 5);
		IST = ist;
	}
	void Initialize(byte *IDT_address)
	{
		gates = (Gate *)IDT_address;

		// build idt
		{
			// exception interrupts
			gates[0x00].setInterruptGate(IntEntryPoints::isr_0, 1, 0);
			gates[0x01].setInterruptGate(IntEntryPoints::isr_1, 1, 0);
			gates[0x02].setInterruptGate(IntEntryPoints::isr_2, 1, 0);
			gates[0x03].setInterruptGate(IntEntryPoints::isr_3, 1, 0);
			gates[0x04].setInterruptGate(IntEntryPoints::isr_4, 1, 0);
			gates[0x05].setInterruptGate(IntEntryPoints::isr_5, 1, 0);
			gates[0x06].setInterruptGate(IntEntryPoints::isr_6, 1, 0);
			gates[0x07].setInterruptGate(IntEntryPoints::isr_7, 1, 0);
			gates[0x08].setInterruptGate(IntEntryPoints::isr_8, 1, 0);
			gates[0x09].setInterruptGate(IntEntryPoints::isr_9, 1, 0);
			gates[0x0a].setInterruptGate(IntEntryPoints::isr_a, 1, 0);
			gates[0x0b].setInterruptGate(IntEntryPoints::isr_b, 1, 0);
			gates[0x0c].setInterruptGate(IntEntryPoints::isr_c, 1, 0);
			gates[0x0d].setInterruptGate(IntEntryPoints::isr_d, 1, 0);
			gates[0x0e].setInterruptGate(IntEntryPoints::isr_e, 1, 0);
			gates[0x0f].setInterruptGate(IntEntryPoints::isr_f, 1, 0);
			gates[0x10].setInterruptGate(IntEntryPoints::isr_10, 1, 0);
			gates[0x11].setInterruptGate(IntEntryPoints::isr_11, 1, 0);
			gates[0x12].setInterruptGate(IntEntryPoints::isr_12, 1, 0);
			gates[0x13].setInterruptGate(IntEntryPoints::isr_13, 1, 0);
			gates[0x14].setInterruptGate(IntEntryPoints::isr_14, 1, 0);
			gates[0x15].setInterruptGate(IntEntryPoints::isr_15, 1, 0);
			gates[0x16].setInterruptGate(IntEntryPoints::isr_16, 1, 0);
			gates[0x17].setInterruptGate(IntEntryPoints::isr_17, 1, 0);
			gates[0x18].setInterruptGate(IntEntryPoints::isr_18, 1, 0);
			gates[0x19].setInterruptGate(IntEntryPoints::isr_19, 1, 0);
			gates[0x1a].setInterruptGate(IntEntryPoints::isr_1a, 1, 0);
			gates[0x1b].setInterruptGate(IntEntryPoints::isr_1b, 1, 0);
			gates[0x1c].setInterruptGate(IntEntryPoints::isr_1c, 1, 0);
			gates[0x1d].setInterruptGate(IntEntryPoints::isr_1d, 1, 0);
			gates[0x1e].setInterruptGate(IntEntryPoints::isr_1e, 1, 0);
			gates[0x1f].setInterruptGate(IntEntryPoints::isr_1f, 1, 0);

			// interrupt requests
			gates[0x20].setInterruptGate(IntEntryPoints::irq_0, 1, 0);
			gates[0x21].setInterruptGate(IntEntryPoints::irq_1, 1, 0);
			gates[0x22].setInterruptGate(IntEntryPoints::irq_2, 1, 0);
			gates[0x23].setInterruptGate(IntEntryPoints::irq_3, 1, 0);
			gates[0x24].setInterruptGate(IntEntryPoints::irq_4, 1, 0);
			gates[0x25].setInterruptGate(IntEntryPoints::irq_5, 1, 0);
			gates[0x26].setInterruptGate(IntEntryPoints::irq_6, 1, 0);
			gates[0x27].setInterruptGate(IntEntryPoints::irq_7, 1, 0);
			gates[0x28].setInterruptGate(IntEntryPoints::irq_8, 1, 0);
			gates[0x29].setInterruptGate(IntEntryPoints::irq_9, 1, 0);
			gates[0x2a].setInterruptGate(IntEntryPoints::irq_10, 1, 0);
			gates[0x2b].setInterruptGate(IntEntryPoints::irq_11, 1, 0);
			gates[0x2c].setInterruptGate(IntEntryPoints::irq_12, 1, 0);
			gates[0x2d].setInterruptGate(IntEntryPoints::irq_13, 1, 0);
			gates[0x2e].setInterruptGate(IntEntryPoints::irq_14, 1, 0);
			gates[0x2f].setInterruptGate(IntEntryPoints::irq_15, 1, 0);

			// os callback
			gates[0x30].setInterruptGate(IntEntryPoints::isr_30, 1, 3);
		}

		// load idt
		IDT_descriptor descriptor((qword)(gates), sizeof(Gate) * IDT_LENGTH - 1);
		loadidt(&descriptor);

		// init pic
		irqOffset = 0x20;
		PIC::Initialize(irqOffset);

		// init irqHandler list
		irqHandlers = new std::vector<IrqHandler>[16];

		kernelPaging = &PageMapLevel4::getCurrent();
	}
	void CleanUp()
	{
		delete[] irqHandlers;
	}

	const char *exceptionMessages[0x20] =
		{
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

	extern "C" qword getCR2();
	extern "C" void irqHandler(registers_t &regs, qword irq_no, bool spurious);
	bool checkSpurious(registers_t &regs)
	{
		word irqs = PIC::getISR();
		// cout << irqs;
		for (int i = 0; i < 16; i++)
		{
			if (irqs & 1 && i != 2)
			{
				irqHandler(regs, i, true);
				return true;
			}

			irqs >>= 1;
		}
		return false;
	}
	extern "C" void exceptionHandler(registers_t &regs, qword int_no, qword err_no)
	{
		// if (int_no == 7 || int_no == 6)
		if (checkSpurious(regs))
		{
			// cout << "Coming from ISR " << int_no << '\n';
			return;
		}

		word irqIsr = PIC::getISR();
		if (irqIsr)
			cout << "Could be an irq: " << std::ostream::base::bin << irqIsr << '\n';
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
	extern "C" void irqHandler(registers_t &regs, qword irq_no, bool spurious)
	{
		if (irq_no != 0 &&
			irq_no != 1 &&
			irq_no != 11)
		{
			cout << (spurious ? "SIRQ: " : "IRQ: ") << irq_no << '\n';
		}

		for (auto handler : irqHandlers[irq_no])
			handler(regs);

		PIC::EndOfInterrupt(irq_no);
	}

	void registerIrqHandler(byte irq_no, IrqHandler handler)
	{
		disableInterrupts();
		irqHandlers[irq_no].push_back(handler);
		enableInterrupts();
	}
}