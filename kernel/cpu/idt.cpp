#include "idt.h"
#include "pic.h"
#include "../drivers/screen.h"
#include "../utils/string.h"
#include "../utils/iostream.h"
#include "../core/sys.h"

using namespace std;

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
extern "C" void isr_30();

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
IDT::Gate *gates = (IDT::Gate *)0x1000;
voidf irqHandlers[16];
byte irqOffset;

void IDT::Gate::setInterruptGate(voidf funOffset)
{
	qword offset = (qword)funOffset;
	offsetLow = offset & 0xffff;
	offsetMid = (offset >> 16) & 0xffff;
	offsetHigh = offset >> 32;
	segmentSelector = KERNEL_CS;
	flags |= 0b10001110;
}
void IDT::Gate::setTrapGate(voidf funOffset)
{

	qword offset = (qword)funOffset;
	offsetLow = offset & 0xffff;
	offsetMid = (offset >> 16) & 0xffff;
	offsetHigh = offset >> 32;
	segmentSelector = KERNEL_CS;
	flags |= 0b10001111;
}
void IDT::Initialize()
{
	// build idt
	// exception interrupts
	gates[0x00].setInterruptGate(isr_0);
	gates[0x01].setInterruptGate(isr_1);
	gates[0x02].setInterruptGate(isr_2);
	gates[0x03].setInterruptGate(isr_3);
	gates[0x04].setInterruptGate(isr_4);
	gates[0x05].setInterruptGate(isr_5);
	gates[0x06].setInterruptGate(isr_6);
	gates[0x07].setInterruptGate(isr_7);
	gates[0x08].setInterruptGate(isr_8);
	gates[0x09].setInterruptGate(isr_9);
	gates[0x0a].setInterruptGate(isr_a);
	gates[0x0b].setInterruptGate(isr_b);
	gates[0x0c].setInterruptGate(isr_c);
	gates[0x0d].setInterruptGate(isr_d);
	gates[0x0e].setInterruptGate(isr_e);
	gates[0x0f].setInterruptGate(isr_f);
	gates[0x10].setInterruptGate(isr_10);
	gates[0x11].setInterruptGate(isr_11);
	gates[0x12].setInterruptGate(isr_12);
	gates[0x13].setInterruptGate(isr_13);
	gates[0x14].setInterruptGate(isr_14);
	gates[0x15].setInterruptGate(isr_15);
	gates[0x16].setInterruptGate(isr_16);
	gates[0x17].setInterruptGate(isr_17);
	gates[0x18].setInterruptGate(isr_18);
	gates[0x19].setInterruptGate(isr_19);
	gates[0x1a].setInterruptGate(isr_1a);
	gates[0x1b].setInterruptGate(isr_1b);
	gates[0x1c].setInterruptGate(isr_1c);
	gates[0x1d].setInterruptGate(isr_1d);
	gates[0x1e].setInterruptGate(isr_1e);
	gates[0x1f].setInterruptGate(isr_1f);

	// interrupt requests
	gates[0x20].setInterruptGate(irq_0);
	gates[0x21].setInterruptGate(irq_1);
	gates[0x22].setInterruptGate(irq_2);
	gates[0x23].setInterruptGate(irq_3);
	gates[0x24].setInterruptGate(irq_4);
	gates[0x25].setInterruptGate(irq_5);
	gates[0x26].setInterruptGate(irq_6);
	gates[0x27].setInterruptGate(irq_7);
	gates[0x28].setInterruptGate(irq_8);
	gates[0x29].setInterruptGate(irq_9);
	gates[0x2a].setInterruptGate(irq_10);
	gates[0x2b].setInterruptGate(irq_11);
	gates[0x2c].setInterruptGate(irq_12);
	gates[0x2d].setInterruptGate(irq_13);
	gates[0x2e].setInterruptGate(irq_14);
	gates[0x2f].setInterruptGate(irq_15);

	// os callback
	gates[0x30].setInterruptGate(isr_30);

	// load idt
	IDT_descriptor descriptor((qword)(gates), sizeof(Gate) * IDT_LENGTH - 1);
	loadidt(&descriptor);

	// descriptorStatic.setOffset((qword)(&gates));
	// descriptorStatic.setSize(sizeof(gates) - 1);
	// loadidt(&descriptorStatic);

	// init pic
	irqOffset = 0x20;
	PIC::Initialize(irqOffset);
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

struct registers_t
{
	qword rax, rcx, rdx, rdi, rsi, r8, r9,
		fs, gs, rbp,
		rip, cs, rflags, rsp, ss;
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
	if (int_no == 7 || int_no == 6)
		if (checkSpurious(regs))
			return;

	word irqIsr = PIC::getISR();
	if (irqIsr)
		cout << "Could be an irq: " << ostream::base::bin << irqIsr << '\n';
	cout
		<< "Exception: " << exceptionMessages[int_no] << " (0x" << ostream::base::hex << int_no
		<< ")\nInfo:\n   Return address: " << regs.cs << ':' << (void *)regs.rip
		<< "\n   Return file offset: " << regs.rip - 0x8000 + 0x200 << ostream::base::dec << '\n';

	switch (int_no)
	{
	case 0xd:
		cout << "Error code: " << err_no << '\n';

		System::blueScreen();
	case 0xe:
		cout << "Error code: " << err_no
			 << "\nExecuting address: " << (void *)regs.rip << '\n';
		displayMemoryRow((byte *)regs.rip);
		cout << "Accessing address: " << (void *)getCR2() << '\n';

		for (qword *rbp = (qword *)regs.rbp; rbp; rbp = (qword *)rbp[0])
		{
			cout << "\nCaller Stack frame: " << (void *)rbp[0]
				 << "   Return address: " << (void *)rbp[1] << " (file offset: " << (void *)(rbp[1] - 0x8000 + 0x200) << ")\n";
		}

		System::blueScreen();
	}
}
bool irq1Recd = false;
void IDT::waitForIrq1()
{
	irq1Recd = false;
	while (!irq1Recd)
		;
}
extern "C" void irqHandler(registers_t &regs, qword irq_no, bool spurious)
{
	if (irq_no == 1)
	{
		irq1Recd = true;
	}
	if (irq_no != 0 && irq_no != 1)
	{
		Screen::print(spurious ? "SIRQ: " : "IRQ: ");
		char str[5];
		byteToHexString(str, irq_no);
		Screen::print(str);
		Screen::print("\n");
	}
	if (irqHandlers[irq_no])
		irqHandlers[irq_no]();
	PIC::EndOfInterrupt(irq_no);
}

extern "C" qword getRSP();
extern "C" qword getRBP();
extern "C" void os_serviceHandler(registers_t &regs)
{
	qword currRsp = getRBP();
	cout << "Breakpoint reached\n";
	bool keepGoing = true;
	enableInterrupts();
	while (keepGoing)
	{
		switch (Keyboard::getKeyPressedEvent().getKeyCode())
		{
		case Keyboard::KeyEvent::KeyCode::C:
			keepGoing = false;
			break;
		case Keyboard::KeyEvent::KeyCode::S:
			constexpr int bytesPerRow = 16;
			// for (byte *i = (byte *)regs.rsp; i <= (byte *)regs.rbp; i += bytesPerRow)
			for (byte *i = (byte *)currRsp; i <= (byte *)regs.rbp; i += bytesPerRow)
				displayMemoryRow(i);
			cout << "RBP: " << (void *)regs.rbp << "   RSP: " << (void *)regs.rsp << "   Current RSP: " << (void *)currRsp << '\n';
			break;
		}
	}
	disableInterrupts();
}

void IDT::registerIrqHandler(byte irq_no, voidf handler)
{
	irqHandlers[irq_no] = handler;
}