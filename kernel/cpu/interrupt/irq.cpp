#include "irq.h"
#include "pic.h"
#include <vector.h>
#include "../../utils/isriostream.h"
#include "../../core/sys.h"

using namespace std;

namespace IRQ
{
	constexpr int irqOffset = 0x20;

	vector<IrqHandler> *irqHandlers;

	void Initialize()
	{
		// init pic
		PIC::Initialize(irqOffset);

		// init irqHandler list
		irqHandlers = new vector<IrqHandler>[16];
	}
	void CleanUp()
	{
		delete[] irqHandlers;
	}

	extern "C" void irqHandler(registers_t &regs, qword irq_no, bool spurious)
	{
		if (irq_no != 0 &&
			irq_no != 1 &&
			irq_no != 11)
		{
			ISR::std::cout << (spurious ? "SIRQ: " : "IRQ: ") << irq_no << '\n';
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