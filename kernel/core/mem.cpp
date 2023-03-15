#include "mem.h"
#include "../utils/iostream.h"
#include "paging.h"
#include "../cpu/idt.h"
#include "sys.h"
#include "../utils/math.h"

using namespace std;

byte &Memory::mapLength = *(byte *)0x5000,
	 &Memory::mapEntrySize = *(byte *)0x5001;
Memory::MapEntry *Memory::memoryMap = (MapEntry *)(0x5000) + 1;

const char *typeToString(Memory::RegionType type)
{
	switch (type)
	{
	case Memory::RegionType::usable:
		return "usable";
	case Memory::RegionType::reserved:
		return "reserved";
	case Memory::RegionType::acpiReclaimable:
		return "ACPI reclaimable";
	case Memory::RegionType::acpiNvs:
		return "ACPI NVS";
	default:
		return "bad";
	}
}

void Memory::swapEntries(byte i, byte j)
{
	Memory::MapEntry temp = memoryMap[i];
	memoryMap[i] = memoryMap[j];
	memoryMap[j] = temp;
}
void Memory::removeEntry(byte i)
{
	mapLength--;
	for (byte k = i; k < mapLength; k++)
		memoryMap[k] = memoryMap[k + 1];
}
void Memory::insertEntry(byte i, byte *start, qword len, RegionType type, uint acpiExtendableAttributes)
{
	for (byte k = mapLength; k > i; k--)
		memoryMap[k] = memoryMap[k - 1];
	mapLength++;
	memoryMap[i].base_address = start;
	memoryMap[i].length = len;
	memoryMap[i].type = type;
	memoryMap[i].acpiExtendableAttributes = acpiExtendableAttributes;
}

void Memory::sortMap()
{
	// sort memory regions
	for (byte l = mapLength - 1; l > 0; l--)
	{
		for (byte i = 0; i < l; i++)
		{
			if (memoryMap[i].base_address > memoryMap[i + 1].base_address)
				swapEntries(i, i + 1);
		}
	}

	// mark unrecognized types as reserved
	for (byte i = 0; i < mapLength; i++)
		if (memoryMap[i].type >= RegionType::limit)
			memoryMap[i].type = RegionType::reserved;

	// merge adjacent regions of the same type
	// mark overlapping regions as the most restrictive type
	// mark unlisted regions as reserved
	for (byte i = 0; i < mapLength - 1; i++)
	{
		MapEntry &m1 = memoryMap[i],
				 &m2 = memoryMap[i + 1];
		if (m1.base_address + m1.length == m2.base_address)
		{
			// no gap between the regions
			if (m1.type == m2.type)
			{
				m1.length += m2.length;
				removeEntry(i + 1);
				i--;
			}
		}
		else if (m1.base_address + m1.length > m2.base_address)
		{
			// regions are overlapping
			if (m1.type == m2.type)
			{
				m1.length = m2.base_address - m1.base_address + m2.length;
				removeEntry(i + 1);
				i--;
			}
			else
			{
				RegionType order[] = {
					RegionType::bad,
					RegionType::reserved,
					RegionType::acpiNvs,
					RegionType::acpiReclaimable,
					RegionType::usable,
				};
				for (byte i = 0; i < 5; i++)
				{
					RegionType currType = order[i];
					if (m1.type == currType || m2.type == currType)
					{
						qword gapLen = m1.length - (m2.base_address - m1.base_address);
						if (m1.type == currType)
						{
							// overlap belongs to m1, shrink m2
							m2.length -= gapLen;
							m2.base_address += gapLen;
						}
						else
						{
							// overlap belongs to m2, shrink m1
							m1.length -= gapLen;
						}
					}
				}
				i--;
			}
		}
		else
		{
			// gap between regions
			insertEntry(i + 1, m1.base_address + m1.length, m2.base_address - m1.base_address - m1.length, RegionType::reserved, 0);
			i--;
		}
	}

	// maybe page align them?

	// page map them
}

void Memory::DisplayMap()
{
	cout << "Memory map: (" << mapLength << " x " << mapEntrySize << "bytes): ";
	for (byte i = 0; i < mapLength; i++)
	{
		MapEntry &en = memoryMap[i];
		cout << "\n[" << i << "]  Base address: " << (void *)en.base_address
			 << "\n     Length: " << (void *)en.length << " (" << en.length
			 << ")\n     End address: " << (void *)(en.base_address + en.length - 1)
			 << "\n     Region type: " << typeToString(en.type) << " (" << (uint)en.type
			 << ")\n     Reserved: " << en.acpiExtendableAttributes;
		// IDT::waitForIrq1();
	}
	cout << '\n';
}
void Memory::Initialize()
{
	// return;
	sortMap();

	int i = 0;
	for (i = 0; i < mapLength; i++)
		if (memoryMap[i].type == RegionType::usable && memoryMap[i].base_address != 0x0)
			break;
	// cout << "Selected usable memory map entry: " << i << '\n';
	MapEntry &entry = memoryMap[i];

	PageMapLevel4 *pml4 = (PageMapLevel4 *)entry.base_address;
	qword nextFree = (qword)pml4 + 0x1000;
	pml4->clearAll();

	// identity map the first 2MB of ram
	// pml4->mapRegion(nextFree, 0x1000, 0x1000, 0x200000 - 0x1000);
	pml4->mapRegion(nextFree, 0, 0, 0x200000);
	pml4->setAsCurrent();

	// identity map the rest of the ram region
	sqword leftToMap = (sqword)entry.base_address + entry.length - 0x200000;
	if (leftToMap > 0)
		pml4->mapRegion(nextFree, 0x200000, 0x200000, leftToMap);

	qword diff = nextFree - (qword)entry.base_address;
	entry.base_address += diff;
	entry.length -= diff;

	Heap::selectHeap(Heap::build(entry.base_address, entry.length));
	// Heap::selectHeap(Heap::build(entry.base_address + diff, entry.length - diff));
}

// Allocator::AllocatorEntry *Allocator::memHeader = nullptr;
// uint Allocator::memHeaderEntryCount = 0;

inline Memory::Heap::AllocatorEntry *Memory::Heap::AllocatorEntry::build(void *address, qword allocationSize, AllocatorEntry *prevAllocation_, AllocatorEntry *nextAllocation_)
{
	AllocatorEntry *obj = (AllocatorEntry *)address;
	obj->prevAllocation = prevAllocation_;
	obj->nextAllocation = nextAllocation_;
	obj->allocatedSize = allocationSize;
	obj->magicCheckStart = magicNumberStart;
	obj->magicCheckEnd() = magicNumberEnd;
#ifdef ALLOC_DBG_MSG
	cout << "Allocated " << allocationSize << " (+" << maintenanceSize << ") bytes at " << obj->getAllocatedBlock() << '\n';
#endif
	return obj;
}
inline int &Memory::Heap::AllocatorEntry::magicCheckEnd() { return *(int *)((byte *)(this + 1) + allocatedSize); }
inline void *Memory::Heap::AllocatorEntry::getSpaceAfter() { return (byte *)this + maintenanceSize + allocatedSize; }
inline void *Memory::Heap::AllocatorEntry::getAllocatedBlock() { return this + 1; }

inline bool Memory::Heap::AllocatorEntry::Corrupted() { return magicCheckStart != magicNumberStart || magicCheckEnd() != magicNumberEnd; }
inline bool Memory::Heap::AllocatorEntry::free()
{
	if (Corrupted())
	{
		cout << "Heap corruption detected while trying to deallocate " << getAllocatedBlock() << '\n';
		cout << ostream::base::hex;
		if (magicCheckStart != magicNumberStart)
			cout << "Found start signature " << magicCheckStart << " instead of " << magicNumberStart << '\n';
		if (magicCheckEnd() != magicNumberEnd)
			cout << "Found end signature " << magicCheckEnd() << " instead of " << magicNumberEnd << '\n';
		cout << ostream::base::dec;
		return false;
	}

	magicCheckStart = 0;
	magicCheckEnd() = 0;
	if (this->prevAllocation)
		prevAllocation->nextAllocation = this->nextAllocation;
	if (this->nextAllocation)
		nextAllocation->prevAllocation = this->prevAllocation;
#ifdef ALLOC_DBG_MSG
	cout << "Freed " << allocatedSize << " (+" << maintenanceSize << ") bytes at " << getAllocatedBlock() << '\n';
#endif
	return true;
}

inline void *Memory::Heap::heapStart() { return (void *)(this + 1); }
// allocationSize is assumed to be a multiple of alignment
inline bool Memory::Heap::fitsAllocation(void *&start, void *end, qword allocationSize, ull alignment)
{
	ull alignedStart = alignValueUpwards((ull)start + AllocatorEntry::headerSize, alignment) - AllocatorEntry::headerSize;
	// from "if ((ull)end - alignedStart >= allocationSize + AllocatorEntry::maintenanceSize)",
	//		rearrange so that there are no subtractions, since there are only unsigned numbers
	if ((ull)end >= allocationSize + AllocatorEntry::maintenanceSize + alignedStart)
	{
		start = (void *)alignedStart;
		return true;
	}
	return false;
}

inline void Memory::Heap::selectHeap(Heap *heap) { selected = heap; }
inline Memory::Heap *Memory::Heap::build(void *address, qword size)
{
	Heap *obj = (Heap *)address;
	obj->heapSize = size - sizeof(Heap);
	obj->firstAllocation = nullptr;
	obj->lastAllocation = nullptr;
	return obj;
}
inline qword Memory::Heap::getSize() { return heapSize; }
inline ull Memory::Heap::getAllocationCount()
{
	ull c = 0;
	for (AllocatorEntry *i = firstAllocation; i; i = i->nextAllocation)
		c++;
	return c;
}

Memory::Heap *Memory::Heap::selected = nullptr;
void *Memory::Heap::Allocate(qword allocationSize, ull alignment)
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

	return nullptr;
}
inline void Memory::Heap::Deallocate(void *ptr)
{
	AllocatorEntry *obj = (AllocatorEntry *)ptr - 1;
	if (!obj->free())
	{
		DisplyMemoryBlock((byte *)obj, 0x100);
		System::blueScreen();
	}
	if (obj == firstAllocation)
		firstAllocation = obj->nextAllocation;
	if (obj == lastAllocation)
		lastAllocation = obj->prevAllocation;
}

inline void *Memory::Heap::AllocateFromSelected(qword allocationSize, ull alignment)
{
#ifdef ALLOC_DBG_MSG
	cout << "Allocating " << allocationSize << " bytes\n";
#endif
	return selected ? selected->Allocate(allocationSize, alignment) : nullptr;
}
inline void Memory::Heap::DeallocateFromSelected(void *ptr)
{
#ifdef ALLOC_DBG_MSG
	cout << "Freeing block at " << ptr << '\n';
#endif
	selected->Deallocate(ptr);
}
inline void Memory::Heap::DeallocateFromSelected(void *ptr, ull size)
{
#ifdef ALLOC_DBG_MSG
	cout << "Freeing block at " << ptr << ", expected length is " << size << " bytes\n";
#endif
	selected->Deallocate(ptr);
}
ull Memory::Heap::getAllocationCountFromSelected()
{
	return selected->getAllocationCount();
}

void *Memory::Allocate(ull size, ull alignment) { return Heap::AllocateFromSelected(size, alignment); }

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