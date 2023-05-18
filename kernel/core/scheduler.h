#pragma once
#include "task.h"

namespace Scheduler
{
	enum class preemptReason
	{
		timeSliceEnded,
		startedSleeping,
		waitingIO,
		taskExited
	};

	void Initialize(Task *terminalTask);
	void CleanUp();

	void add(Task *task);

	void preempt(registers_t &regs, preemptReason reason);
	// void finish(registers_t &regs);

	void tick(registers_t &regs);
	void irqReceived(int irq_no);

	void sleep(registers_t &regs, ull untilTime);
	void waitForIrq(registers_t &regs, IDT::Irq_no irq_no);
	void waitForTask(registers_t &regs, Task *task);

	Task *getCurrentTask();
}