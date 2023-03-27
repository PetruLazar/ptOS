#pragma once
#include "task.h"

namespace Scheduler
{
	void Initialize(Task *terminalTask, Task *idleTask);
	void CleanUp();

	void add(Task *task);

	void preempt(registers_t &regs, bool erase);
	// void finish(registers_t &regs);
}