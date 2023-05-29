#pragma once
#include "../cpu/idt.h"
#include <string.h>

class Task
{
	registers_t regs;
	byte *pageSpace, *fileContent, *stack, *heap;
	std::vector<byte *> programResources;
	bool m_isKernelTask;

public:
	class TaskInfo
	{
	public:
		ull data;

		inline TaskInfo() {}
		inline TaskInfo(ull data) : data(data) {}
	} taskInfo;

	inline Task(const registers_t &regs, bool isKernelTask = false, byte *pageSpace = nullptr, byte *fileContent = nullptr, byte *stack = nullptr, byte *heap = nullptr)
		: regs(regs), pageSpace(pageSpace), fileContent(fileContent), stack(stack), heap(heap), m_isKernelTask(isKernelTask) {}
	inline ~Task()
	{
		for (auto ptr : programResources)
			delete[] ptr;
		if (pageSpace)
			delete[] pageSpace;
		if (fileContent)
			delete[] fileContent;
		if (stack)
			delete[] stack;
		if (heap)
			delete[] heap;
	}

	static Task *createTask(const std::string16 &executableFileName);

	inline static void switchContext(Task *currentTask, Task *targetTask, registers_t &regs)
	{
		// save the state of the currentTask
		if (currentTask)
			currentTask->regs = regs;
		// switch context to the selected task
		regs = targetTask->regs;
		// enable interrupts for the new task
		regs.rflags |= 1 << 9;
	}

	inline registers_t &getRegs() { return regs; }
	inline bool isKernelTask() { return m_isKernelTask; }

	inline void bindResource(byte *resource) { programResources.push_back(resource); }
};