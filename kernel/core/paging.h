#pragma once
#include <types.h>

class PageEntry
{
protected:
	qword value;

public:
	static constexpr qword addressMask = 0x000ffffffffff000,
						   presentBit = 1 << 0,
						   writeAccessBit = 1 << 1,
						   userPageBit = 1 << 2,
						   accessedBit = 1 << 5;

	inline void set(qword address, bool writeAccess = true, bool userPage = false)
	{
		value = address & addressMask | presentBit;
		if (writeAccess)
			value |= writeAccessBit;
		if (userPage)
			value |= userPageBit;
	}
	inline void clear() { value = 0; }

	inline void setAddress(qword address)
	{
		value = value & (~addressMask) | (address & addressMask);
	}
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

	inline qword getAddress() { return value & addressMask; }
	inline bool isPresent() { return value & presentBit; }
	inline bool isWriteAccess() { return value & writeAccessBit; }
	inline bool isUserPage() { return value & userPageBit; }
	inline bool isAccessed() { return value & accessedBit; }
};

class PageTableEntry : public PageEntry
{
	static constexpr qword dirtyBit = 1 << 6;

public:
	PageTableEntry() {}

	inline bool isDirty() { return value & dirtyBit; }
};
class PageTable
{
public:
	PageTableEntry entries[512];

	inline void clearAll()
	{
		for (int i = 0; i < 512; i++)
			entries[i].clear();
	}
};

class PageDirectoryEntry : public PageEntry
{
	static constexpr qword pageSizeBit = 1 << 7;

public:
	PageTable *getTable() { return (PageTable *)getAddress(); }

	inline void setPageSize(bool big) { big ? value |= pageSizeBit : value &= ~pageSizeBit; }

	inline bool isPageBig() { return value & pageSizeBit; }
};
class PageDirectory
{
public:
	inline PageTable *getOrCreate(word index, qword &freeSpace)
	{
		if (entries[index].isPresent())
			return entries[index].getTable();
		entries[index].set(freeSpace);
		freeSpace += 0x1000;
		PageTable *ret = entries[index].getTable();
		ret->clearAll();
		return ret;
	}

	PageDirectoryEntry entries[512];

	inline void clearAll()
	{
		for (int i = 0; i < 512; i++)
			entries[i].clear();
	}
};

class PageDirectoryPointerTableEntry : public PageEntry
{
	static constexpr qword pageSizeBit = 1 << 7;

public:
	PageDirectory *getTable() { return (PageDirectory *)getAddress(); }

	inline void setPageSize(bool big) { big ? value |= pageSizeBit : value &= ~pageSizeBit; }

	inline bool isPageBig() { return value & pageSizeBit; }
};
class PageDirectoryPointerTable
{
public:
	inline PageDirectory *getOrCreate(word index, qword &freeSpace)
	{
		if (entries[index].isPresent())
			return entries[index].getTable();
		entries[index].set(freeSpace);
		freeSpace += 0x1000;
		PageDirectory *ret = entries[index].getTable();
		ret->clearAll();
		return ret;
	}

	PageDirectoryPointerTableEntry entries[512];

	inline void clearAll()
	{
		for (int i = 0; i < 512; i++)
			entries[i].clear();
	}
};

class PageMapLevel4Entry : public PageEntry
{
public:
	PageDirectoryPointerTable *getTable() { return (PageDirectoryPointerTable *)getAddress(); }
};
class PageMapLevel4
{
public:
	inline PageDirectoryPointerTable *getOrCreate(word index, qword &freeSpace)
	{
		if (entries[index].isPresent())
			return entries[index].getTable();
		entries[index].set(freeSpace);
		freeSpace += 0x1000;
		PageDirectoryPointerTable *ret = entries[index].getTable();
		ret->clearAll();
		return ret;
	}

	PageMapLevel4Entry entries[512];

	inline void clearAll()
	{
		for (int i = 0; i < 512; i++)
			entries[i].clear();
	}

	void mapRegion(qword &freeSpace, qword virtualAddress, qword physicalAddress, qword len);
	void unmapRegion(qword virtualAddress, qword len);

	bool getPhysicalAddress(qword virtualAddress, qword &physicalAddress);

	static PageMapLevel4 &getCurrent();
	void setAsCurrent();
};

void PagingTest();