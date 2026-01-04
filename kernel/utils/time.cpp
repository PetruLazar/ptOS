#include "time.h"
#include "../cpu/interrupt/irq.h"
#include "../core/scheduler.h"
#include "../cpu/interrupt/pit.h"

#include <iostream.h>
using namespace std;

namespace Time
{
	typedef IRQ::IrqHandler InterruptHandlerCallback;
	typedef ull (*TimeGetterCallback)();

	struct TimerConfiguration
	{
		InterruptHandlerCallback interruptHandler;
		TimeGetterCallback timeGetter;
	};

	const TimerConfiguration supportedTimers[] = {
		{ // PIT
			PIT::InterruptHandler,
			PIT::driver_time,
		},
		{ // APICtimer
			nullptr,
			nullptr,
		},
		{ // HPET
			nullptr,
			nullptr,
		},
	};
	const TimerConfiguration *activeTimerConfiguration = nullptr;

	void IrqHandler(registers_t &regs)
	{
		if (activeTimerConfiguration)
			activeTimerConfiguration->interruptHandler(regs);
		Scheduler::tick(regs);
	}
	void SelectTimer(TimerSource timerSource)
	{
		if (timerSource < TimerSource::noTimer)
		{
			activeTimerConfiguration = &supportedTimers[(byte)timerSource];
		}
		else
		{
			activeTimerConfiguration = nullptr;
		}
	}

	void Initialize()
	{
		IRQ::registerIrqHandler(0, IrqHandler);
	}

	qword driver_time()
	{
		return activeTimerConfiguration->timeGetter();
	}
}