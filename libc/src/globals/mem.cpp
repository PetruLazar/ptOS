#include <mem.h>
#include <iostream.h>

using namespace std;

namespace Memory
{
	Heap *selectedHeap;

	void Heap::AllocatorEntry::CorruptionDetected()
	{
		cout << "Heap corruption detected while trying to deallocate " << getAllocatedBlock() << '\n';
		cout << ostream::base::hex;
		if (magicCheckStart != magicNumberStart)
			cout << "Found start signature " << magicCheckStart << " instead of " << magicNumberStart << '\n';
		if (magicCheckEnd() != magicNumberEnd)
			cout << "Found end signature " << magicCheckEnd() << " instead of " << magicNumberEnd << '\n';
		cout << ostream::base::dec;
		cout << "Memory dump:\n";
		DisplyMemoryBlock((byte *)this, 0x100);
	}

	void Heap::displayAllocationSummary()
	{
		for (auto *i = firstAllocation; i; i = i->nextAllocation)
		{
			// cheat to get the allocation size
			ull size = ((ull *)i)[2];
			cout << "Allocation of " << size << " bytes at " << (void *)i << ", data at " << (void *)i->getAllocatedBlock() << ":\n";
			DisplyMemoryBlock((byte *)i->getAllocatedBlock(), 0x30);
		}
	}
	void *Heap::Allocate(qword allocationSize, ull alignment)
	{
		allocationSize = alignValueUpwards(allocationSize, alignment);
		void *alignedStart;

		// if there is no allocation, try to begin filling the heap
		if (!firstAllocation)
		{
			alignedStart = heapStart();
			if (!fitsAllocation(alignedStart, (byte *)alignedStart + heapSize, allocationSize, alignment))
				return nullptr;
			AllocatorEntry *obj = AllocatorEntry::build(alignedStart, allocationSize, nullptr, nullptr);
			firstAllocation = obj;
			lastAllocation = obj;
			return obj->getAllocatedBlock();
		}

		// try before the first
		alignedStart = heapStart();
		if (fitsAllocation(alignedStart, firstAllocation, allocationSize, alignment))
		{
			AllocatorEntry *obj = AllocatorEntry::build(alignedStart, allocationSize, nullptr, firstAllocation);
			firstAllocation->prevAllocation = obj;
			firstAllocation = obj;
			return obj->getAllocatedBlock();
		}

		// try inbetween all the entries
		for (AllocatorEntry *i = firstAllocation; i != lastAllocation; i = i->nextAllocation)
		{
			alignedStart = i->getSpaceAfter();
			if (fitsAllocation(alignedStart, i->nextAllocation, allocationSize, alignment))
			{
				AllocatorEntry *obj = AllocatorEntry::build(alignedStart, allocationSize, i, i->nextAllocation);
				i->nextAllocation = obj;
				obj->nextAllocation->prevAllocation = obj;
				return obj->getAllocatedBlock();
			}
		}

		// try after the last, if there is no space, return nullptr
		alignedStart = lastAllocation->getSpaceAfter();
		if (fitsAllocation(alignedStart, (byte *)heapStart() + heapSize, allocationSize, alignment))
		{
			AllocatorEntry *obj = AllocatorEntry::build(alignedStart, allocationSize, lastAllocation, nullptr);
			lastAllocation->nextAllocation = obj;
			lastAllocation = obj;
			return obj->getAllocatedBlock();
		}

		cout << "Warning: Memory full!\n";
		return nullptr;
	}
}

void *malloc(ull size) { return Memory::Heap::AllocateFromSelected(size, 0x10); }
void *calloc(ull size)
{
	int *ptr = (int *)Memory::Heap::AllocateFromSelected(size, 0x10);
	size >>= 2;
	for (ull i = 0; i < size; i++)
		ptr[i] = 0;
	return ptr;
}
void free(void *block) { Memory::Heap::DeallocateFromSelected(block); }

void *operator new(size_t size) { return Memory::Heap::AllocateFromSelected(size, 0x10); }
void *operator new[](size_t size) { return Memory::Heap::AllocateFromSelected(size, 0x10); }
void operator delete(void *ptr) { Memory::Heap::DeallocateFromSelected(ptr); }
void operator delete(void *ptr, size_t size) { Memory::Heap::DeallocateFromSelected(ptr, size); }
void operator delete[](void *ptr) { Memory::Heap::DeallocateFromSelected(ptr); }
void operator delete[](void *ptr, size_t size) { Memory::Heap::DeallocateFromSelected(ptr); }