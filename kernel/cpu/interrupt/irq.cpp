#include "irq.h"
#include "apic.h"
#include "pic.h"
#include "pit.h"
#include <vector.h>
#include "../../utils/isriostream.h"
#include "../../core/sys.h"
#include "../../utils/time.h"
#include "../../debug/verbose.h"

using namespace std;

namespace IRQ
{
	constexpr int irqOffset = 0x20;

	vector<IrqHandler> *irqHandlers;

	void Initialize()
	{
		// init pic
		VERBOSE_LOG("Initializing PIC...\n");
		PIC::Initialize(irqOffset);
		if (APIC::DetectPresence())
		{
			VERBOSE_LOG("Initializing APIC...\n");
			APIC::Initialize();
			PIC::Disable(); // does nothing yet
		}
		else
		{
			VERBOSE_LOG("Initializing PIT...\n");
			Time::SelectTimer(Time::TimerSource::PIT);
			PIT::ConfigureChannel(PIT::SelectChannel::channel0, PIT::OperatingMode::rateGenerator, 1000 / ms_per_timeint);
		}

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
	extern "C" void irqApicHandler(registers_t &regs, qword irq_no)
	{
		ISR::std::cout << "APIC INT " << irq_no << '\n';
	}

	void registerIrqHandler(byte irq_no, IrqHandler handler)
	{
		disableInterrupts();
		irqHandlers[irq_no].push_back(handler);
		enableInterrupts();
	}
	void unregisterIrqHandler(byte irq_no, IrqHandler handler)
	{
		disableInterrupts();
		auto &vec = irqHandlers[irq_no];
		auto len = vec.getSize();
		for (ull i = 0; i < len; i++)
		{
			if (vec[i] == handler)
			{
				vec.erase(i);
				break;
			}
		}
		enableInterrupts();
	}
}