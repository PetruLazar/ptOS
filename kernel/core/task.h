#pragma once
#include "../cpu/idt.h"
#include <string.h>

class Thread;
#include "thread.h"

class Task
{
	byte *pageSpace, *programImage, *heap;
	std::vector<byte *> programResources;
	Thread *mainThread = nullptr;
	int threadCount = 0;
	dword pageAllocationMap;
	bool m_isKernelTask, m_isDead = false;

public:
	inline Task(bool isKernelTask = false, byte *pageSpace = nullptr, dword pageAllocationMap = 0xffff0000, byte *programImage = nullptr, byte *heap = nullptr)
		: pageSpace(pageSpace), programImage(programImage), heap(heap), m_isKernelTask(isKernelTask), pageAllocationMap(pageAllocationMap)
	{
	}
	inline ~Task()
	{
		for (auto ptr : programResources)
			delete[] ptr;
		if (pageSpace)
			delete[] pageSpace;
		if (programImage)
			delete[] programImage;
		if (heap)
			delete[] heap;
	}

	static Task *createTask(const std::string16 &executableFileName);

	inline bool isKernelTask() { return m_isKernelTask; }
	inline Thread *getMainThread() { return mainThread; }
	inline bool isDead() { return m_isDead; }
	inline void kill() { m_isDead = true; }

	inline void bindResource(byte *resource) { programResources.push_back(resource); }

	friend Thread;
};