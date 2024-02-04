#pragma once

class Task;
#include "task.h"

class Thread
{
	Task *parentTask;
	byte *stack;

	registers_t regs;

public:
	class ThreadInfo
	{
	public:
		ull data;

		inline ThreadInfo() {}
		inline ThreadInfo(ull data) : data(data) {}
	} threadInfo;

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

	Task *getParentTask() { return parentTask; }
	inline registers_t &getRegs() { return regs; }
	bool IsMainThread();
};