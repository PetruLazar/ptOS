#include "task.h"
#include "filesystem/filesystem.h"
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
		// cout << "Could not read file: " << Filesystem::resultAsString(res) << "\n";
		return nullptr;
	}
	byte *pageSpace = (byte *)Memory::Allocate(0x10000, 0x1000),
		 *stack = (byte *)Memory::Allocate(0x10000, 0x1000),
		 *heap = (byte *)Memory::Allocate(0x10000, 0x1000);
	dword pageAllocationMap = 0xffff0000;

	PageMapLevel4 *paging = PageMapLevel4::create(pageSpace, pageAllocationMap);
	bool mappingFailed = false;
	if (paging != nullptr)
	{
		if (!paging->mapRegion(pageSpace, pageAllocationMap, 0x100000, (qword)content, alignValueUpwards(len, 0x1000), PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::userPageBit))) // page loaded code
			mappingFailed = true;
		if (!paging->mapRegion(pageSpace, pageAllocationMap, 0x40000, (ull)stack, 0x10000, PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::userPageBit))) // page stack
			mappingFailed = true;
		if (!paging->mapRegion(pageSpace, pageAllocationMap, 0x7f0000000000, (ull)heap, 0x10000, PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::userPageBit))) // page heap
			mappingFailed = true;
		if (!paging->mapRegion(pageSpace, pageAllocationMap, 0xFFFFFFFF80000000, 0x0000, 0x80000, PageEntry::EntryAttributes(PageEntry::writeAccessBit))) // page kernel
			mappingFailed = true;
		// get interrupt stack physical address and map it
		ull interruptStackPhysical;
		if (!paging->getCurrent().getPhysicalAddress(0xFFFFFFFF80080000, interruptStackPhysical, false))
			mappingFailed = true;
		if (!paging->mapRegion(pageSpace, pageAllocationMap, 0xFFFFFFFF80080000, interruptStackPhysical, 0x10000, PageEntry::EntryAttributes(PageEntry::writeAccessBit)))
			mappingFailed = true;
	}

	if (paging == nullptr || mappingFailed)
	{
		cout << "Ran out of space for the paging structure.\n";
		delete[] pageSpace;
		delete[] content;
		delete[] stack;
		delete[] heap;
		return nullptr;
	}
	ExecutableFileHeader *header = (ExecutableFileHeader *)content;
	registers_t regs;
	regs.rip = header->entryPoint < 0x100000 ? 0x100000 : header->entryPoint;
	regs.cs = GDT::USER_CS | 3;
	regs.cr3 = paging;
	regs.rbp = regs.rsp = 0x50000;
	regs.fs = regs.gs = regs.ss = GDT::USER_DS | 3;
	regs.rflags = 0;
	Task *task = new Task(false, pageSpace, pageAllocationMap, content, heap);
	Thread *thread = new Thread(task, regs, stack);
	return task;
}