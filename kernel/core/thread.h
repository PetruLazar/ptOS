#pragma once

class Task;
#include "task.h"

union ThreadActivationCondition
{
	// maybe support for sleeping with a timeout?
	// so that this union can be removed
	Thread *blockedBy;
	ull sleepingUntil;
};

class Thread
{
	Task *parentTask;
	byte *stack;

	registers_t regs;

	ThreadActivationCondition activationCondition;

public:
	Thread(Task *parentTask, const registers_t &regs, byte *stack = nullptr);
	~Thread();

	inline static void switchContext(Thread *currentThread, Thread *targetThread, registers_t &regs)
	{
		// save the state of the currentTask
		if (currentThread)
			currentThread->regs = regs;
		// switch context to the selected task
		regs = targetThread->regs;
		// enable interrupts for the new task
		regs.rflags |= 1 << 9;
	}

	inline Task *getParentTask() { return parentTask; }
	inline registers_t &getRegs() { return regs; }
	bool IsMainThread();

	inline void block(Thread *blocker) { activationCondition.blockedBy = blocker; }
	inline Thread *getBlocker() { return activationCondition.blockedBy; }

	inline void sleepUntil(ull targetTime) { activationCondition.sleepingUntil = targetTime; }
	inline bool finishedSleeping(ull currentTime) { return activationCondition.sleepingUntil <= currentTime; }
};