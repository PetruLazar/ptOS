#include "time.h"
#include "../cpu/idt.h"
#include "../core/scheduler.h"

#include "iostream.h"
using namespace std;

namespace Time
{
	static constexpr int preempt_time = 5;

	qword irqCount = 0;
	word preempt_timer;

	void IrqHandler(registers_t &regs)
	{
		irqCount++;
		preempt_timer--;
		if (!preempt_timer)
		{
			Scheduler::preempt(regs, false);
			preempt_timer = preempt_time;
			// cout << ostream::base::hex << regs.cs << ':' << (void *)regs.rip << ostream::base::dec << '\n';
		}
	}

	void Initialize()
	{
		IDT::registerIrqHandler(0, IrqHandler);
		preempt_timer = preempt_time;
	}

	qword time()
	{
		return irqCount;
	}
}