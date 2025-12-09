#include "time.h"
#include "../cpu/interrupt/irq.h"
#include "../core/scheduler.h"

#include <iostream.h>
using namespace std;

namespace Time
{
	qword irqCount = 0;

	void IrqHandler(registers_t &regs)
	{
		irqCount++;
		Scheduler::tick(regs);
	}

	void Initialize()
	{
		IRQ::registerIrqHandler(0, IrqHandler);
	}

	qword driver_time()
	{
		return irqCount * IRQ::ms_per_timeint;
	}
}