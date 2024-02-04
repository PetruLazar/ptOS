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

	void enable();
	void disable();
	bool isEnabled();

	void Initialize(Task *terminalTask);
	void CleanUp();

	void add(Thread *thread);

	void kill(Task *task, int returnedValue);
	void kill(Thread *thread, int returnedValue);
	void preempt(registers_t &regs, preemptReason reason);
	// void finish(registers_t &regs);

	void tick(registers_t &regs);
	void irqReceived(int irq_no);

	void sleep(registers_t &regs, ull untilTime);
	void waitForIrq(registers_t &regs, IDT::Irq_no irq_no);
	void waitForThread(registers_t &regs, Thread *thread);

	Thread *getCurrentThread();
}