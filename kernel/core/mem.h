#pragma once
#include <types.h>
#include <iostream.h>

// #define ALLOC_DBG_MSG

class Memory
{
public:
	enum class RegionType : uint32_t
	{
		usable = 1,
		reserved,
		acpiReclaimable,
		acpiNvs,
		bad,

		limit
	};

private:
	class MapEntry
	{
	public:
		byte *base_address;
		qword length;
		RegionType type;
		uint acpiExtendableAttributes;
	} static *memoryMap;

	static byte &mapLength, &mapEntrySize;

	static void swapEntries(byte i, byte j);
	static void removeEntry(byte i);
	static void insertEntry(byte i, byte *start, qword len, RegionType type, uint acpiExtendableAttributes);

	static void sortMap();

public:
	static void DisplayMap();
	static void Initialize();

	class Heap
	{
		class AllocatorEntry
		{
		public:
			AllocatorEntry *prevAllocation, *nextAllocation;

		private:
			qword allocatedSize;

			static constexpr int magicNumberStart = 0x514a2b04;
			static constexpr int magicNumberEnd = 0xa94b5fcb;
			int reserved;
			int magicCheckStart;

			inline int &magicCheckEnd();

		public:
			static constexpr qword headerSize = sizeof(AllocatorEntry *) * 2 + sizeof(qword) + sizeof(int) * 2,
								   trailerSize = sizeof(int),
								   maintenanceSize = headerSize + trailerSize;
			inline static AllocatorEntry *build(void *address, qword allocationSize, AllocatorEntry *prevAllocation_, AllocatorEntry *nextAllocation_);

			inline void *getSpaceAfter();
			inline void *getAllocatedBlock();

			inline bool Corrupted();
			inline bool free();
		};

		qword heapSize;
		AllocatorEntry *firstAllocation, *lastAllocation;

		inline bool fitsAllocation(void *&start, void *end, qword allocationSize, ull alignment);
		inline void *heapStart();
		static Heap *selected;

	public:
		inline static void selectHeap(Heap *heap);
		inline static Heap *build(void *address, qword size);

		inline qword getSize();
		ull getAllocationCount();
		void displayAllocationSummary();

		void *Allocate(qword allocationSize, ull alignment);
		inline void Deallocate(void *ptr);

		inline static void *AllocateFromSelected(qword allocationSize, ull alignment);
		inline static void DeallocateFromSelected(void *ptr);
		inline static void DeallocateFromSelected(void *ptr, ull size);

		inline static ull getAllocationCountFromSelected() { return selected->getAllocationCount(); }
		inline static void displayAllocationSummaryFromSelected() { return selected->displayAllocationSummary(); }
	};

	static void *Allocate(ull size, ull alignment);
};

void *malloc(ull size);
void *calloc(ull size);
void free(void *block);

extern "C" void memcpy(void *dest, const void *src, ull len);
extern "C" void memmove(void *dest, const void *src, ull len);
extern "C" void memset(void *ptr, ull len, byte val);

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *ptr);
void operator delete(void *ptr, size_t size);
void operator delete[](void *ptr);
void operator delete[](void *ptr, size_t size);