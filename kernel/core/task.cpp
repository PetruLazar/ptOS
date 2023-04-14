#include "task.h"
#include "filesystem.h"
#include "mem.h"
#include <iostream.h>
#include <math.h>
#include "../cpu/gdt.h"

using namespace std;

struct ExecutableFileHeader
{
	ull entryPoint;
};

Task *Task::createTask(const std::string16 &executableFileName)
{
	byte *content;
	ull len;
	Filesystem::result res = Filesystem::ReadFile(executableFileName, content, len);
	if (res != Filesystem::result::success)
	{
		cout << "Could not read file: " << Filesystem::resultAsString(res) << "\n";
		return nullptr;
	}
	byte *pageSpace = (byte *)Memory::Allocate(0x10000, 0x1000),
		 *stack = (byte *)Memory::Allocate(0x10000, 0x1000);
	PageMapLevel4 *paging = (PageMapLevel4 *)pageSpace;
	paging->clearAll();
	qword freeSpace = (qword)(paging + 0x1);
	paging->mapRegion(freeSpace, 0x100000, (qword)content, alignValueUpwards(len, 0x1000), true, true); // page loaded code
	paging->mapRegion(freeSpace, 0x40000, (ull)stack, 0x10000, true, true);								// page stack
	paging->mapRegion(freeSpace, 0x1000, 0x1000, 0x40000 - 0x1000, false, false);						// page kernel
	if (freeSpace > (qword)(pageSpace + 0x10000))
	{
		cout << "Ran out of space for the paging structure.\n";
		delete[] pageSpace;
		delete[] content;
		delete[] stack;
		return nullptr;
	}
	ExecutableFileHeader *header = (ExecutableFileHeader *)content;
	registers_t regs;
	regs.rip = header->entryPoint < 0x100000 ? 0x100000 : header->entryPoint;
	regs.cs = GDT::USER_CS | 3;
	regs.cr3 = paging;
	regs.rbp = regs.rsp = 0x50000;
	// regs.ds =
	regs.fs = regs.gs = regs.ss = GDT::USER_DS | 3;
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