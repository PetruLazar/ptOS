#pragma once
#include <types.h>

class PageTable;
class PageDirectory;
class PageDirectoryPointerTable;
class PageMapLevel4;

class PageEntry
{
public:
	static constexpr qword
		addressMask_4kb = 0x000ffffffffff000,
		addressMask_2mb = 0x000fffffffe00000,
		addressMask_1gb = 0x000fffffc0000000,

		entriesPerTable = 512,

		presentBit = 1 << 0,

		writeAccessBit = 1 << 1,
		userPageBit = 1 << 2,
		pageWriteThroughBit = 1 << 3,
		pageCacheDisable = 1 << 4,

		accessedBit = 1 << 5,
		dirtyBit = 1 << 6,

		pageSizeBit = 1 << 7;

	class EntryAttributes
	{
		static constexpr byte attributeMask = PageEntry::writeAccessBit | PageEntry::userPageBit | PageEntry::pageWriteThroughBit | PageEntry::pageCacheDisable;

		byte reserved1 : 1;
	public:
		byte writeAccess : 1;
		byte userPage : 1;
		byte pageWriteThrough : 1;
		byte pageCacheDisable : 1;

	private:
		byte reserved2: 3;

		inline void set(byte bitmask)
		{
			*(byte*)this = bitmask & attributeMask;
		}

	public:
		inline EntryAttributes(byte bitmask = 0)
		{
			set(bitmask);
		}
		inline byte get()
		{
			return (*(byte*)this) & attributeMask;
		}
		inline bool operator==(EntryAttributes other)
		{
			return get() == other.get();
		}
	};

protected:
	qword value;

	inline void set(qword maskedAddress, EntryAttributes attributes)
	{
		value = maskedAddress | presentBit | attributes.get();
	}

	inline void setAddress(qword maskedAddress)
	{
		value = value & (~addressMask_4kb) | maskedAddress;
	}

public:

	inline void clear() { value = 0; }

	inline void setPresent(bool present)
	{
		present ? value |= presentBit : value &= ~presentBit;
	}
	inline void clearAccessed()
	{
		value &= ~accessedBit;
	}

	inline EntryAttributes &attributes() { return *(EntryAttributes*)&value; }
	inline byte &attributesByte() { return *(byte*)&value; }
	inline qword getAddress(qword mask = addressMask_4kb) { return value & mask; }
	inline bool isPresent() { return value & presentBit; }
	inline bool isAccessed() { return value & accessedBit; }
};

class PageTableEntry : public PageEntry
{
public:
	inline void set(qword physicalAddress, EntryAttributes attributes) { PageEntry::set(physicalAddress & addressMask_4kb, attributes); }

	inline void clearDirty() { value &= ~dirtyBit; }

	inline bool isDirty() { return value & dirtyBit; }
};
class PageDirectoryEntry : public PageEntry
{
public:
	PageTable *getTable() { return (PageTable *)getAddress(); }

	inline void set(qword physicalAddress, EntryAttributes attributes)
	{
		PageEntry::set(physicalAddress & addressMask_2mb, attributes);
		value |= PageEntry::pageSizeBit;
	}
	inline void set(PageTable *table) { PageEntry::set((qword)table & addressMask_4kb, EntryAttributes(writeAccessBit | userPageBit)); }

	bool expand(void *pageSpace, dword &pageAllocationMap);
	inline void setPageSize(bool big) { big ? value |= pageSizeBit : value &= ~pageSizeBit; }
	inline void clearDirty() { value &= ~dirtyBit; }

	inline bool isPageBig() { return value & pageSizeBit; }
	inline bool isDirty() { return value & dirtyBit; }
	inline bool isAccessed() { return value & accessedBit; }
};
class PageDirectoryPointerTableEntry : public PageEntry
{
public:
	PageDirectory *getTable() { return (PageDirectory *)getAddress(); }

	inline void set(qword physicalAddress, EntryAttributes attributes)
	{
		PageEntry::set(physicalAddress & addressMask_1gb, attributes);
		value |= PageEntry::pageSizeBit;
	}
	inline void set(PageDirectory *table) { PageEntry::set((qword)table & addressMask_4kb, EntryAttributes(writeAccessBit | userPageBit)); }

	bool expand(void *pageSpace, dword &pageAllocationMap);
	inline void setPageSize(bool big) { big ? value |= pageSizeBit : value &= ~pageSizeBit; }
	inline void clearDirty() { value &= ~dirtyBit; }

	inline bool isPageBig() { return value & pageSizeBit; }
	inline bool isDirty() { return value & dirtyBit; }
};
class PageMapLevel4Entry : public PageEntry
{
public:
	PageDirectoryPointerTable *getTable() { return (PageDirectoryPointerTable *)getAddress(); }

	inline void set(PageDirectoryPointerTable *table) { PageEntry::set((qword)table & addressMask_4kb, EntryAttributes(writeAccessBit | userPageBit)); }
};

class PageTable
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 12;

	PageTableEntry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);

	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(qword virtualAddress, qword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(qword virtualAddress, qword len);
	// check if this region can be collapsed to a big page in the parent table
	bool canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	void clearAll(void *pageSpace, dword &pageAllocationMap);
};
class PageDirectory
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 21;

	PageDirectoryEntry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);

	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(dword virtualAddress, dword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(dword virtualAddress, dword len);
	// check if this region can be collapsed to a big page in the parent table
	bool canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	void clearAll(void *pageSpace, dword &pageAllocationMap);
};
class PageDirectoryPointerTable
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 30;

	PageDirectoryPointerTableEntry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);

	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(dword virtualAddress, dword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(dword virtualAddress, dword len);
	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	void clearAll(void *pageSpace, dword &pageAllocationMap);
};
class PageMapLevel4
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 39;

	PageMapLevel4Entry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, PageEntry::EntryAttributes attributes);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);
	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(dword virtualAddress, dword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(dword virtualAddress, dword len);

	void clearAll(void *pageSpace, dword &pageAllocationMap);

	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	static PageMapLevel4 *create(void *pageSpace, dword &pageAllocationMap);
	inline static PageMapLevel4 &getCurrent()
	{
		PageMapLevel4 *retVal;
		asm volatile(
			"mov %[pml4], cr3"
			: [pml4] "=r"(retVal));
		return *retVal;
	}
	inline void setAsCurrent()
	{
		asm volatile(
			"mov cr3, %[pml4]"
			:
			: [pml4] "r"(this));
	}
};

void PagingTest();