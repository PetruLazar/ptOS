#pragma once
#include "types.h"
#include "math.h"

namespace Memory
{
	class Heap;
	extern Heap *selectedHeap;
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

			inline int &magicCheckEnd() { return *(int *)((byte *)(this + 1) + allocatedSize); }

		public:
			static constexpr qword headerSize = sizeof(AllocatorEntry *) * 2 + sizeof(qword) + sizeof(int) * 2,
								   trailerSize = sizeof(int),
								   maintenanceSize = headerSize + trailerSize;

			inline static AllocatorEntry *build(void *address, qword allocationSize, AllocatorEntry *prevAllocation_, AllocatorEntry *nextAllocation_)
			{
				AllocatorEntry *obj = (AllocatorEntry *)address;
				obj->prevAllocation = prevAllocation_;
				obj->nextAllocation = nextAllocation_;
				obj->allocatedSize = allocationSize;
				obj->magicCheckStart = magicNumberStart;
				obj->magicCheckEnd() = magicNumberEnd;
				return obj;
			}

			inline void *getSpaceAfter() { return (byte *)this + maintenanceSize + allocatedSize; }
			inline void *getAllocatedBlock() { return this + 1; }

			void CorruptionDetected();
			inline bool Corrupted() { return magicCheckStart != magicNumberStart || magicCheckEnd() != magicNumberEnd; }
			bool free()
			{
				if (Corrupted())
				{
					CorruptionDetected();
					return false;
				}

				magicCheckStart = 0;
				magicCheckEnd() = 0;
				if (this->prevAllocation)
					prevAllocation->nextAllocation = this->nextAllocation;
				if (this->nextAllocation)
					nextAllocation->prevAllocation = this->prevAllocation;
				return true;
			}
		};

		qword heapSize;
		AllocatorEntry *firstAllocation, *lastAllocation;

		// allocationSize is assumed to be a multiple of alignment
		inline bool fitsAllocation(void *&start, void *end, qword allocationSize, ull alignment)
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

		void CorruptionDetected(void *corruptedAllocation);

		inline void *heapStart() { return (void *)(this + 1); }

	public:
		inline static Heap *build(void *address, qword size)
		{
			Heap *obj = (Heap *)address;
			obj->heapSize = size - sizeof(Heap);
			obj->firstAllocation = nullptr;
			obj->lastAllocation = nullptr;
			return obj;
		}

		inline qword getSize() { return heapSize; }
		inline ull getAllocationCount()
		{
			ull c = 0;
			for (AllocatorEntry *i = firstAllocation; i; i = i->nextAllocation)
				c++;
			return c;
		}
		void displayAllocationSummary();

		void *Allocate(qword allocationSize, ull alignment);
		void Deallocate(void *ptr)
		{
			AllocatorEntry *obj = (AllocatorEntry *)ptr - 1;
			obj->free();
			if (obj == firstAllocation)
				firstAllocation = obj->nextAllocation;
			if (obj == lastAllocation)
				lastAllocation = obj->prevAllocation;
		}

		inline static void *AllocateFromSelected(qword allocationSize, ull alignment) { return selectedHeap ? selectedHeap->Allocate(allocationSize, alignment) : nullptr; }
		inline static void DeallocateFromSelected(void *ptr) { selectedHeap->Deallocate(ptr); }
		inline static void DeallocateFromSelected(void *ptr, ull size) { selectedHeap->Deallocate(ptr); }

		inline static ull getAllocationCountFromSelected() { return selectedHeap->getAllocationCount(); }
		inline static void displayAllocationSummaryFromSelected() { return selectedHeap->displayAllocationSummary(); }
	};

	inline void *Allocate(ull size, ull alignment) { return Heap::AllocateFromSelected(size, alignment); }
};

void *malloc(ull size);
void *calloc(ull size);
void free(void *block);

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *ptr);
void operator delete(void *ptr, size_t size);
void operator delete[](void *ptr);
void operator delete[](void *ptr, size_t size);