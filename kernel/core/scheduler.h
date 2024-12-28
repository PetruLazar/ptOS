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

	void Initialize();
	void CleanUp();

	void add(Thread *thread);

	void kill(Task *task, int returnedValue);
	void kill(Thread *thread, int returnedValue);
	void preempt(registers_t &regs, preemptReason reason);
	// void finish(registers_t &regs);

	void tick(registers_t &regs);

	void sleep(registers_t &regs, ull untilTime);
	bool waitForThread(registers_t &regs, Thread *thread);
	void unblockThread(Thread *blockingThread, Thread *blockedThread);

	Thread *getCurrentThread();
}