#pragma once
#include <types.h>

class PageTable;
class PageDirectory;
class PageDirectoryPointerTable;
class PageMapLevel4;

class PageEntry
{
protected:
	qword value;

	inline void set(qword maskedAddress, bool writeAccess, bool userPage)
	{
		value = maskedAddress | presentBit;
		if (writeAccess)
			value |= writeAccessBit;
		if (userPage)
			value |= userPageBit;
	}

	inline void setAddress(qword maskedAddress)
	{
		value = value & (~addressMask_4kb) | maskedAddress;
	}

public:
	// PWT, PCD and PAT bits are not supported in the current implementation

	static constexpr qword
		addressMask_4kb = 0x000ffffffffff000,
		addressMask_2mb = 0x000fffffffe00000,
		addressMask_1gb = 0x000fffffc0000000,

		entriesPerTable = 512,

		presentBit = 1 << 0,
		writeAccessBit = 1 << 1,
		userPageBit = 1 << 2,
		accessedBit = 1 << 5,
		dirtyBit = 1 << 6,
		pageSizeBit = 1 << 7;

	inline void clear() { value = 0; }

	inline void setPresent(bool present)
	{
		present ? value |= presentBit : value &= ~presentBit;
	}
	inline void setWriteAccess(bool writeAccess)
	{
		writeAccess ? value |= writeAccessBit : value &= ~writeAccessBit;
	}
	inline void setUserPage(bool userPage)
	{
		userPage ? value |= userPageBit : value &= ~userPageBit;
	}
	inline void clearAccessed()
	{
		value &= ~accessedBit;
	}

	inline qword getAddress(qword mask = addressMask_4kb) { return value & mask; }
	inline bool isPresent() { return value & presentBit; }
	inline bool isWriteAccess() { return value & writeAccessBit; }
	inline bool isUserPage() { return value & userPageBit; }
	inline bool isAccessed() { return value & accessedBit; }
};

class PageTableEntry : public PageEntry
{
public:
	inline void set(qword physicalAddress, bool writeAccess, bool userPage) { PageEntry::set(physicalAddress & addressMask_4kb, writeAccess, userPage); }

	inline void clearDirty() { value &= ~dirtyBit; }

	inline bool isDirty() { return value & dirtyBit; }
};
class PageDirectoryEntry : public PageEntry
{
public:
	PageTable *getTable() { return (PageTable *)getAddress(); }

	inline void set(qword physicalAddress, bool writeAccess, bool userPage) { PageEntry::set(physicalAddress & addressMask_2mb, writeAccess, userPage); value |= PageEntry::pageSizeBit; }
	inline void set(PageTable* table) { PageEntry::set((qword)table & addressMask_4kb, true, true); }

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

	inline void set(qword physicalAddress, bool writeAccess, bool userPage) { PageEntry::set(physicalAddress & addressMask_1gb, writeAccess, userPage); value |= PageEntry::pageSizeBit; }
	inline void set(PageDirectory* table) {PageEntry::set((qword)table & addressMask_4kb, true, true); }

	bool expand(void* pageSpace, dword& pageAllocationMap);
	inline void setPageSize(bool big) { big ? value |= pageSizeBit : value &= ~pageSizeBit; }
	inline void clearDirty() { value &= ~dirtyBit; }

	inline bool isPageBig() { return value & pageSizeBit; }
	inline bool isDirty() { return value & dirtyBit; }
};
class PageMapLevel4Entry : public PageEntry
{
public:
	PageDirectoryPointerTable *getTable() { return (PageDirectoryPointerTable *)getAddress(); }

	inline void set(PageDirectoryPointerTable* table) {PageEntry::set((qword)table & addressMask_4kb, true, true); }
};

class PageTable
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 12;

	PageTableEntry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);

	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(qword virtualAddress, qword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(qword virtualAddress, qword len);
	// check if this region can be collapsed to a big page in the parent table
	bool canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
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
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);

	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(dword virtualAddress, dword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(dword virtualAddress, dword len);
	// check if this region can be collapsed to a big page in the parent table
	bool canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	void clearAll(void *pageSpace, dword &pageAllocationMap);
};
class PageDirectoryPointerTable
{
public:
	static constexpr qword bytesPerEntry = (qword)1 << 30;
	// inline PageDirectory *getOrCreate(word index, qword &freeSpace, bool writeAccess, bool userPage)
	// {
	// 	if (entries[index].isPresent())
	// 		return entries[index].getTable();
	// 	entries[index].set(freeSpace, writeAccess, userPage);
	// 	freeSpace += 0x1000;
	// 	PageDirectory *ret = entries[index].getTable();
	// 	ret->clearAll();
	// 	return ret;
	// }

	PageDirectoryPointerTableEntry entries[PageEntry::entriesPerTable];

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
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

	// inline PageDirectoryPointerTable *getOrCreate(word index, qword &freeSpace, bool writeAccess, bool userPage)
	// {
	// 	if (entries[index].isPresent())
	// 		return entries[index].getTable();
	// 	entries[index].set(freeSpace, writeAccess, userPage);
	// 	freeSpace += 0x1000;
	// 	PageDirectoryPointerTable *ret = entries[index].getTable();
	// 	ret->clearAll();
	// 	return ret;
	// }

	PageMapLevel4Entry entries[PageEntry::entriesPerTable];

	// void mapRegion(qword &freeSpace, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
	// void unmapRegion(qword virtualAddress, qword len);

	// bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	// map a region of virtual space to a continuous block of physical space
	bool mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage);
	// completely unmap a region of virtual space
	bool unmapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword len);
	// check if region has any dirty page; unmapped pages are not considered dirty
	bool dirtyRegion(dword virtualAddress, dword len);
	// check if region has any accessed page; unmapped pages are not considered accessed
	bool accessedRegion(dword virtualAddress, dword len);

	void clearAll(void *pageSpace, dword &pageAllocationMap);

	// get the physical address that a virtual address translates to
	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess = false);

	static PageMapLevel4* create(void *pageSpace, dword &pageAllocationMap);
	static PageMapLevel4 &getCurrent();
	void setAsCurrent();
};

void PagingTest();