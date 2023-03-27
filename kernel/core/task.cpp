#include "task.h"
#include "filesystem.h"
#include "mem.h"
#include <iostream.h>
#include <math.h>

using namespace std;

Task *Task::createTask(const std::string16 &executableFileName)
{
	byte *content;
	ull len;
	if (!Filesystem::ReadFile(executableFileName, content, len))
	{
		cout << "Could not read file\n";
		return nullptr;
	}
	byte *pageSpace = (byte *)Memory::Allocate(0x10000, 0x1000),
		 *stack = (byte *)Memory::Allocate(0x10000, 0x1000);
	PageMapLevel4 *paging = (PageMapLevel4 *)pageSpace;
	paging->clearAll();
	qword freeSpace = (qword)(paging + 0x1);
	paging->mapRegion(freeSpace, 0x1000, 0x1000, 0x30000);									// page kernel
	paging->mapRegion(freeSpace, 0x100000, (qword)content, alignValueUpwards(len, 0x1000)); // page loaded code
	paging->mapRegion(freeSpace, 0x40000, (ull)stack, 0x10000);								// page stack
	if (freeSpace > (qword)(pageSpace + 0x10000))
	{
		cout << "Ran out of space for the paging structure.\n";
		delete[] pageSpace;
		delete[] content;
		delete[] stack;
		return nullptr;
	}

	registers_t regs;
	regs.rip = 0x100000;
	regs.cs = 0x8;
	regs.cr3 = paging;
	regs.rbp = regs.rsp = 0x50000;
	regs.fs = regs.gs = regs.ss = 0x10;
	regs.rflags = 0;
	// cout << "pageSpace: " << (void *)pageSpace
	// 	 << "\ncontent: " << (void *)content
	// 	 << "\nstack: " << (void *)stack << '\n';
	return new Task(regs, pageSpace, content, stack);

	// build virtual space:
	// map kernel into the virtual space
	// map actual image into the virtual space
	// allocate and map stack into the virtual space
}