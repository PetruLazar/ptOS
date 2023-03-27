#pragma once
#include "../cpu/idt.h"
#include "../utils/string.h"

class Task
{
	registers_t regs;
	byte *pageSpace, *fileContent, *stack;

public:
	inline Task(const registers_t &regs, byte *pageSpace = nullptr, byte *fileContent = nullptr, byte *stack = nullptr) : regs(regs), pageSpace(pageSpace), fileContent(fileContent), stack(stack) {}
	inline ~Task()
	{
		if (pageSpace)
			delete[] pageSpace;
		if (fileContent)
			delete[] fileContent;
		if (stack)
			delete[] stack;
	}

	static Task *createTask(const std::string16 &executableFileName);

	inline static void switchContext(Task *currentTask, Task *targetTask, registers_t &regs)
	{
		// save the state of the currentTask
		currentTask->regs = regs;
		// switch context to the selected task
		regs = targetTask->regs;
		// enable interrupts for the new task
		regs.rflags |= 1 << 9;
	}

	inline registers_t &getRegs() { return regs; }
};