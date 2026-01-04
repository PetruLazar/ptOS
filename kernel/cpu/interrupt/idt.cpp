#include "idt.h"
#include "../../utils/isriostream.h"
#include "pic.h"
#include "../../core/sys.h"
#include "../../core/paging.h"
#include "../../core/scheduler.h"
#include "../gdt.h"

using namespace ISR::std;

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

		// local APIC interrupts
		extern "C" void irq_40();
		extern "C" void irq_41();
		extern "C" void irq_42();
		extern "C" void irq_43();
		extern "C" void irq_44();
		extern "C" void irq_45();
		extern "C" void irq_46();
		extern "C" void irq_4f();
	}

	extern "C" void loadidt(void *);
	extern "C" void clearint();

	constexpr int IDT_LENGTH = 256;

	typedef void (*voidf)(); // pointer to void returning function with no args
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

		void setInterruptGate(voidf funOffset, byte ist, byte dpl)
		{
			qword offset = (qword)funOffset;
			offsetLow = offset & 0xffff;
			offsetMid = (offset >> 16) & 0xffff;
			offsetHigh = offset >> 32;
			segmentSelector = GDT::KERNEL_CS;
			flags = 0b10001110 | (dpl << 5);
			IST = ist;
		}
		void setTrapGate(voidf funOffset, byte ist, byte dpl)
		{
			qword offset = (qword)funOffset;
			offsetLow = offset & 0xffff;
			offsetMid = (offset >> 16) & 0xffff;
			offsetHigh = offset >> 32;
			segmentSelector = GDT::KERNEL_CS;
			flags |= 0b10001111 | (dpl << 5);
			IST = ist;
		}
	};

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

	void Build(Gate *gates, bool useIST)
	{
		// build idt
		{
			byte exceptionIST, irqIST, syscallIST;
			if (useIST)
			{
				exceptionIST = 1;
				irqIST = 2;
				syscallIST = 3;
			}
			else
			{
				exceptionIST = 0;
				irqIST = 0;
				syscallIST = 0;
			}

			// exception interrupts
			gates[0x00].setInterruptGate(IntEntryPoints::isr_0, exceptionIST, 0);
			gates[0x01].setInterruptGate(IntEntryPoints::isr_1, exceptionIST, 0);
			gates[0x02].setInterruptGate(IntEntryPoints::isr_2, exceptionIST, 0);
			gates[0x03].setInterruptGate(IntEntryPoints::isr_3, exceptionIST, 0);
			gates[0x04].setInterruptGate(IntEntryPoints::isr_4, exceptionIST, 0);
			gates[0x05].setInterruptGate(IntEntryPoints::isr_5, exceptionIST, 0);
			gates[0x06].setInterruptGate(IntEntryPoints::isr_6, exceptionIST, 0);
			gates[0x07].setInterruptGate(IntEntryPoints::isr_7, exceptionIST, 0);
			gates[0x08].setInterruptGate(IntEntryPoints::isr_8, exceptionIST, 0);
			gates[0x09].setInterruptGate(IntEntryPoints::isr_9, exceptionIST, 0);
			gates[0x0a].setInterruptGate(IntEntryPoints::isr_a, exceptionIST, 0);
			gates[0x0b].setInterruptGate(IntEntryPoints::isr_b, exceptionIST, 0);
			gates[0x0c].setInterruptGate(IntEntryPoints::isr_c, exceptionIST, 0);
			gates[0x0d].setInterruptGate(IntEntryPoints::isr_d, exceptionIST, 0);
			gates[0x0e].setInterruptGate(IntEntryPoints::isr_e, exceptionIST, 0);
			gates[0x0f].setInterruptGate(IntEntryPoints::isr_f, exceptionIST, 0);
			gates[0x10].setInterruptGate(IntEntryPoints::isr_10, exceptionIST, 0);
			gates[0x11].setInterruptGate(IntEntryPoints::isr_11, exceptionIST, 0);
			gates[0x12].setInterruptGate(IntEntryPoints::isr_12, exceptionIST, 0);
			gates[0x13].setInterruptGate(IntEntryPoints::isr_13, exceptionIST, 0);
			gates[0x14].setInterruptGate(IntEntryPoints::isr_14, exceptionIST, 0);
			gates[0x15].setInterruptGate(IntEntryPoints::isr_15, exceptionIST, 0);
			gates[0x16].setInterruptGate(IntEntryPoints::isr_16, exceptionIST, 0);
			gates[0x17].setInterruptGate(IntEntryPoints::isr_17, exceptionIST, 0);
			gates[0x18].setInterruptGate(IntEntryPoints::isr_18, exceptionIST, 0);
			gates[0x19].setInterruptGate(IntEntryPoints::isr_19, exceptionIST, 0);
			gates[0x1a].setInterruptGate(IntEntryPoints::isr_1a, exceptionIST, 0);
			gates[0x1b].setInterruptGate(IntEntryPoints::isr_1b, exceptionIST, 0);
			gates[0x1c].setInterruptGate(IntEntryPoints::isr_1c, exceptionIST, 0);
			gates[0x1d].setInterruptGate(IntEntryPoints::isr_1d, exceptionIST, 0);
			gates[0x1e].setInterruptGate(IntEntryPoints::isr_1e, exceptionIST, 0);
			gates[0x1f].setInterruptGate(IntEntryPoints::isr_1f, exceptionIST, 0);

			// interrupt requests
			gates[0x20].setInterruptGate(IntEntryPoints::irq_0, irqIST, 0);
			gates[0x21].setInterruptGate(IntEntryPoints::irq_1, irqIST, 0);
			gates[0x22].setInterruptGate(IntEntryPoints::irq_2, irqIST, 0);
			gates[0x23].setInterruptGate(IntEntryPoints::irq_3, irqIST, 0);
			gates[0x24].setInterruptGate(IntEntryPoints::irq_4, irqIST, 0);
			gates[0x25].setInterruptGate(IntEntryPoints::irq_5, irqIST, 0);
			gates[0x26].setInterruptGate(IntEntryPoints::irq_6, irqIST, 0);
			gates[0x27].setInterruptGate(IntEntryPoints::irq_7, irqIST, 0);
			gates[0x28].setInterruptGate(IntEntryPoints::irq_8, irqIST, 0);
			gates[0x29].setInterruptGate(IntEntryPoints::irq_9, irqIST, 0);
			gates[0x2a].setInterruptGate(IntEntryPoints::irq_10, irqIST, 0);
			gates[0x2b].setInterruptGate(IntEntryPoints::irq_11, irqIST, 0);
			gates[0x2c].setInterruptGate(IntEntryPoints::irq_12, irqIST, 0);
			gates[0x2d].setInterruptGate(IntEntryPoints::irq_13, irqIST, 0);
			gates[0x2e].setInterruptGate(IntEntryPoints::irq_14, irqIST, 0);
			gates[0x2f].setInterruptGate(IntEntryPoints::irq_15, irqIST, 0);

			// os callback
			gates[0x30].setInterruptGate(IntEntryPoints::isr_30, syscallIST, 3);

			// local APIC
			gates[0x40].setInterruptGate(IntEntryPoints::irq_40, irqIST, 0);
			gates[0x41].setInterruptGate(IntEntryPoints::irq_41, irqIST, 0);
			gates[0x42].setInterruptGate(IntEntryPoints::irq_42, irqIST, 0);
			gates[0x43].setInterruptGate(IntEntryPoints::irq_43, irqIST, 0);
			gates[0x44].setInterruptGate(IntEntryPoints::irq_44, irqIST, 0);
			gates[0x45].setInterruptGate(IntEntryPoints::irq_45, irqIST, 0);
			gates[0x46].setInterruptGate(IntEntryPoints::irq_46, irqIST, 0);
			gates[0x4f].setInterruptGate(IntEntryPoints::irq_4f, irqIST, 0);
		}

		// load idt
		IDT_descriptor descriptor((qword)(gates), sizeof(Gate) * IDT_LENGTH - 1);
		loadidt(&descriptor);
	}

	void PreInitialize(byte *IDT_address)
	{
		Build((Gate*)IDT_address, false);
	}
	void Initialize(byte *IDT_address)
	{
		Build((Gate*)IDT_address, true);
	}
}