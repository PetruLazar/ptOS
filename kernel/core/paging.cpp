#include "paging.h"
#include <iostream.h>

using namespace std;

extern "C" void setCR3(PageMapLevel4 *pml4_ptr);
extern "C" PageMapLevel4 &getCR3();

// in pt: 4kb pages (0x1000)
// in pd: 2mb pages (0x200000)
// in pdpt: 1gb pages (0x40000000)

void *AllocatePage(void *pageSpace, dword &pageAllocationMap)
{
	if (pageAllocationMap == (dword)(-1))
		return nullptr; // no more space to allocate

	for (dword i = 0; i < sizeof(pageAllocationMap) * 8; i++)
	{
		if ((pageAllocationMap & (1 << i)) == 0)
		{
			// free, allocate
			pageAllocationMap |= (1 << i);
			PageTable *table = (PageTable *)((byte *)pageSpace + 0x1000 * i);
			table->clearAll(pageSpace, pageAllocationMap);
			return table;
		}
	}

	return nullptr; // should never happened
}
void DeallocatePage(void *pageSpace, dword &pageAllocationMap, void *table)
{
	// get index
	qword bitIndex = ((qword)table - (qword)pageSpace) / 0x1000;

	// for now, just assume the inputs are correct
	pageAllocationMap &= (dword)(-1) ^ (1 << bitIndex);
}

void getEntryBounds(qword virtualAddress, qword len, word &startEntry, word &endEntry, int bitShift)
{
	const qword virtualAddress_max = virtualAddress + len - 1;
	const int bitShift_parentEntries = bitShift + 9;

	startEntry = (virtualAddress >> bitShift) & 0x1ff;
	if (
		(virtualAddress >> (bitShift + 9)) !=
		((virtualAddress + len - 1) >> (bitShift + 9)))
	{
		// endEntry in different region, snap to max entry
		endEntry = PageEntry::entriesPerTable - 1;
	}
	else
	{
		endEntry = ((virtualAddress + len - 1) >> bitShift) & 0x1ff;
	}
}

bool PageTable::canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	// check the entries against the desired physical address

	// get bounds
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 12);

	// if entries before are not mapped to pages immediately before or
	// entries after are not mapped immediately after, then this table cannot
	// be collapsed by mapping a big page in the parent table

	qword physicalAddress_lower = physicalAddress - bytesPerEntry * startEntry;
	// if lower physical address is not aligned to 2MB, then this cannot be collapsed, even if continuous
	if ((physicalAddress_lower & PageEntry::addressMask_2mb) != physicalAddress_lower)
		return false;

	// check entries before
	for (word i = 0; i < startEntry; i++)
	{
		PageTableEntry &entry = entries[i];
		bool continuous = entry.isPresent() && entry.getAddress() == physicalAddress_lower && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
		if (!continuous)
			return false;

		physicalAddress_lower += bytesPerEntry;
	}

	// check entries after (start outside in, to check most distant entries first, hopefully detecting negative cases quicker)
	qword physicalAddress_upper = physicalAddress + bytesPerEntry * (PageEntry::entriesPerTable - 1 - startEntry);
	for (word i = PageEntry::entriesPerTable - 1; i > endEntry; i--)
	{
		PageTableEntry &entry = entries[i];
		bool continuous = entry.isPresent() && entry.getAddress() == physicalAddress_upper && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
		if (!continuous)
			return false;

		physicalAddress_upper -= bytesPerEntry;
	}

	return true;
}
bool PageTable::mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 12);

	for (word i = startEntry; i <= endEntry; i++)
	{
		PageTableEntry &entry = entries[i];
		// page table entries always point to physical pages, no need to deallocate child tables
		entry.set(physicalAddress, writeAccess, userPage);

		// increment stuff
		physicalAddress += bytesPerEntry;
	}

	return true;
}
void PageTable::clearAll(void *pageSpace, dword &pageAllocationMap)
{
	for (int i = 0; i < PageEntry::entriesPerTable; i++)
		entries[i].clear();
}
bool PageTable::getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess)
{
	PageTableEntry &entry = entries[(virtualAddress >> 12) & 0x1ff];
	if (!entry.isPresent())
		return false;
	physicalAddress = entry.getAddress(PageEntry::addressMask_4kb) | (virtualAddress & (bytesPerEntry - 1));
	return true;
}

bool PageDirectoryEntry::expand(void *pageSpace, dword &pageAllocationMap)
{
	PageTable *table = (PageTable *)AllocatePage(pageSpace, pageAllocationMap);
	if (table == nullptr)
		return false;

	qword physicalAddress = this->getAddress(addressMask_2mb);
	bool userPage = this->isUserPage();
	bool writeAccess = this->isWriteAccess();

	if (!table->mapRegion(pageSpace, pageAllocationMap, 0, physicalAddress, 1 << 21, writeAccess, userPage))
		return false;
	this->set(table);
	return true;
}
bool PageDirectory::canBeCollapsed(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	// check the entries against the desired physical address

	// get bounds
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 21);

	// if lower physical address is not aligned to 1GB, then this cannot be collapsed, even if continuous
	qword physicalAddress_lower = (physicalAddress & PageEntry::addressMask_2mb) - bytesPerEntry * startEntry;
	if ((physicalAddress_lower & PageEntry::addressMask_1gb) != physicalAddress_lower)
		return false;

	// check entries before
	for (word i = 0; i < startEntry; i++)
	{
		PageDirectoryEntry &entry = entries[i];
		// for the entire entry to be continuous, it has to be a big page
		// when this region is affected by mapping, if it were continuous, it would be collapsed into a big page
		// if it is not a big page, it means the region is not continuous
		bool continuous = entry.isPresent() && entry.isPageBig() && entry.getAddress(PageEntry::addressMask_2mb) == physicalAddress_lower && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
		if (!continuous)
			return false;

		physicalAddress_lower += bytesPerEntry;
	}

	// check entries after
	qword physicalAddress_upper = physicalAddress + (bytesPerEntry) * (PageEntry::entriesPerTable - 1 - startEntry);
	for (word i = PageEntry::entriesPerTable - 1; i > endEntry; i--)
	{
		PageDirectoryEntry &entry = entries[i];
		bool continuous = entry.isPresent() && entry.isPageBig() && entry.getAddress(PageEntry::addressMask_2mb) == physicalAddress_upper && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
		if (!continuous)
			return false;

		physicalAddress_upper -= bytesPerEntry;
	}

	// check partial entry before, if any
	if (physicalAddress_lower < physicalAddress)
	{
		// this also covers the case in which this partial entry is the first entry AND the last entry (mapped region is
		// less than an entry and is not aligned to either of the region boundaries)

		// a partial entry can be collapsed if the sub-entries are consecutive
		// OR if the entry is already a big page mapped to the desired physical address
		PageDirectoryEntry &entry = entries[startEntry];
		bool continuous = entry.isPresent() && ((entry.isPageBig() && entry.getAddress(PageEntry::addressMask_2mb) == physicalAddress_lower && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess) ||
												(!entry.isPageBig() && entry.getTable()->canBeCollapsed(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, bytesPerEntry - (physicalAddress - physicalAddress_lower), writeAccess, userPage)));
		if (!continuous)
			return false;
	}
	else if (physicalAddress + len < physicalAddress_upper + bytesPerEntry)
	{
		// at this point there is the last entry was either handled by the "true" branch,
		// or it is not the same as the partial entry before (there could be the same before and after partial entry, if the mapped
		// region is entirely contained within a single entry), if it exists at all

		PageDirectoryEntry &entry = entries[endEntry];
		bool continuous = entry.isPresent() && ((entry.isPageBig() && entry.getAddress(PageEntry::addressMask_2mb) == physicalAddress_upper && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess) ||
												(!entry.isPageBig() && entry.getTable()->canBeCollapsed(pageSpace, pageAllocationMap, virtualAddress + (physicalAddress_upper - physicalAddress), physicalAddress_upper, physicalAddress_upper + bytesPerEntry - (physicalAddress + len), writeAccess, userPage)));
		if (!continuous)
			return false;
	}
}
bool PageDirectory::mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 21);

	for (word i = startEntry; i <= endEntry; i++)
	{
		PageDirectoryEntry &entry = entries[i];

		const qword virtualAddress_entryBase = virtualAddress & ~(bytesPerEntry - 1);

		if (
			virtualAddress != virtualAddress_entryBase || // first entry, partially covered
			len < bytesPerEntry							  // last entry, partially covered
		)
		{
			// partial entry: first, last, or both

			// if big page or not present: either skip mapping if it is already mapped, or expand the entry
			// if not big page: either     pass the operation to the child table, or collapse the entry
			if (entry.isPresent())
			{
				// entry is present
				if (entry.isPageBig())
				{
					// entry present and is big page: expand the entry if mapped to a different region
					// and with the same attributes, otherwise do nothing
					bool matching = (entry.getAddress(PageEntry::addressMask_2mb) == (physicalAddress & PageEntry::addressMask_2mb)) && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
					if (!matching)
					{
						// expand and map the region
						if (entry.expand(pageSpace, pageAllocationMap) == false)
							return false;
						if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
							return false;
					}
				}
				else
				{
					// not a big page. Check if the entry can be collapsed with the new mapping
					PageTable *table = entry.getTable();
					if (table->canBeCollapsed(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					{
						qword physicalAddress_base = table->entries[0].getAddress();
						table->clearAll(pageSpace, pageAllocationMap);
						DeallocatePage(pageSpace, pageAllocationMap, table);
						entry.set(physicalAddress_base, writeAccess, userPage);
					}
					else
					{
						if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
							return false;
					}
				}
			}
			else
			{
				// entry not present
				PageTable *table = (PageTable *)AllocatePage(pageSpace, pageAllocationMap);
				if (table == nullptr)
					return false;
				entry.set(table);
				if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					return false;
			}

			// increment stuff
			qword partialLen = bytesPerEntry - (virtualAddress - virtualAddress_entryBase);
			if (partialLen > len)
				partialLen = len;

			physicalAddress += partialLen;
			virtualAddress += partialLen;
			len -= partialLen;
		}
		else
		{
			// middle, impartial entry

			// if big page or not present: simply replace the current mapping
			// if not big page:            collapse the child table

			bool aligned = (physicalAddress & (bytesPerEntry - 1)) == 0;

			if (entry.isPresent())
			{
				if (aligned)
				{
					// entry is present
					if (!entry.isPageBig())
					{
						// not big page: deallocate first
						PageTable *table = entry.getTable();
						table->clearAll(pageSpace, pageAllocationMap);
						DeallocatePage(pageSpace, pageAllocationMap, table);
					}
					entry.set(physicalAddress, writeAccess, userPage);
				}
				else
				{
					if (entry.isPageBig())
					{
						PageTable *table = (PageTable *)AllocatePage(pageSpace, pageAllocationMap);
						if (table == nullptr)
							return false;
						entry.set(table);
					}
					if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
						return false;
				}
			}
			else
			{
				// entry is not present
				if (aligned)
				{
					entry.set(physicalAddress, writeAccess, userPage);
				}
				else
				{
					PageTable *table = (PageTable *)AllocatePage(pageSpace, pageAllocationMap);
					if (table == nullptr)
						return false;
					entry.set(table);
					if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
						return false;
				}
			}

			// increment stuff
			physicalAddress += bytesPerEntry;
			virtualAddress += bytesPerEntry;
			len -= bytesPerEntry;
		}
	}

	return true;
}
void PageDirectory::clearAll(void *pageSpace, dword &pageAllocationMap)
{
	for (int i = 0; i < PageEntry::entriesPerTable; i++)
	{
		PageDirectoryEntry &entry = entries[i];
		// recursively deallocate child tables
		if (entry.isPresent() && !entry.isPageBig())
		{
			PageTable *table = entry.getTable();
			table->clearAll(pageSpace, pageAllocationMap);
			DeallocatePage(pageSpace, pageAllocationMap, table);
		}
		entry.clear();
	}
}
bool PageDirectory::getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess)
{
	PageDirectoryEntry &entry = entries[(virtualAddress >> 21) & 0x1ff];
	if (!entry.isPresent())
		return false;
	if (!entry.isPageBig())
		return entry.getTable()->getPhysicalAddress(virtualAddress, physicalAddress, isUserAccess);
	physicalAddress = entry.getAddress(PageEntry::addressMask_2mb) | (virtualAddress & (bytesPerEntry - 1));
	return true;
}

bool PageDirectoryPointerTableEntry::expand(void *pageSpace, dword &pageAllocationMap)
{
	PageDirectory *table = (PageDirectory *)AllocatePage(pageSpace, pageAllocationMap);
	if (table == nullptr)
		return false;

	qword physicalAddress = this->getAddress(addressMask_1gb);
	bool userPage = this->isUserPage();
	bool writeAccess = this->isWriteAccess();

	table->mapRegion(pageSpace, pageAllocationMap, 0, physicalAddress, 1 << 30, writeAccess, userPage);
	this->set(table);
	return true;
}
bool PageDirectoryPointerTable::mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 30);

	for (word i = startEntry; i <= endEntry; i++)
	{
		PageDirectoryPointerTableEntry &entry = entries[i];

		const qword virtualAddress_entryBase = virtualAddress & ~(bytesPerEntry - 1);

		if (
			virtualAddress != virtualAddress_entryBase || // first entry, partially covered
			len < bytesPerEntry							  // last entry, partially covered
		)
		{
			// partial entry: first, last, or both
			if (entry.isPresent())
			{
				// entry is present
				if (entry.isPageBig())
				{
					bool matching = (entry.getAddress(PageEntry::addressMask_1gb) == (physicalAddress & PageEntry::addressMask_1gb)) && entry.isUserPage() == userPage && entry.isWriteAccess() == writeAccess;
					if (!matching)
					{
						if (entry.expand(pageSpace, pageAllocationMap) == false)
							return false;
						if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
							return false;
					}
				}
				else
				{
					PageDirectory *table = entry.getTable();
					if (table->canBeCollapsed(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					{
						qword physicalAddress_base = table->entries[0].getAddress();
						table->clearAll(pageSpace, pageAllocationMap);
						DeallocatePage(pageSpace, pageAllocationMap, table);
						entry.set(physicalAddress_base, writeAccess, userPage);
					}
					else
					{
						if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
							return false;
					}
				}
			}
			else
			{
				// entry not present
				PageDirectory *table = (PageDirectory *)AllocatePage(pageSpace, pageAllocationMap);
				if (table == nullptr)
					return false;
				entry.set(table);
				if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					return false;
			}

			// increment stuff
			qword partialLen = bytesPerEntry - (virtualAddress - virtualAddress_entryBase);
			if (partialLen > len)
				partialLen = len;

			physicalAddress += partialLen;
			virtualAddress += partialLen;
			len -= partialLen;
		}
		else
		{
			// middle, impartial entry
			bool aligned = (physicalAddress & (bytesPerEntry - 1)) == 0;

			if (entry.isPresent())
			{
				if (aligned)
				{
					if (!entry.isPageBig())
					{
						PageDirectory *table = entry.getTable();
						table->clearAll(pageSpace, pageAllocationMap);
						DeallocatePage(pageSpace, pageAllocationMap, table);
					}
					entry.set(physicalAddress, writeAccess, userPage);
				}
				else
				{
					if (entry.isPageBig())
					{
						PageDirectory *table = (PageDirectory *)AllocatePage(pageSpace, pageAllocationMap);
						if (table == nullptr)
							return false;
						entry.set(table);
					}
					if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
						return false;
				}
			}
			else
			{
				entry.set(physicalAddress, writeAccess, userPage);
			}

			// increment stuff
			physicalAddress += bytesPerEntry;
			virtualAddress += bytesPerEntry;
			len -= bytesPerEntry;
		}
	}

	return true;
}
void PageDirectoryPointerTable::clearAll(void *pageSpace, dword &pageAllocationMap)
{
	for (int i = 0; i < PageEntry::entriesPerTable; i++)
	{
		PageDirectoryPointerTableEntry &entry = entries[i];
		// recursively deallocate child tables
		if (entry.isPresent() && !entry.isPageBig())
		{
			PageDirectory *table = entry.getTable();
			table->clearAll(pageSpace, pageAllocationMap);
			DeallocatePage(pageSpace, pageAllocationMap, table);
		}
		entries[i].clear();
	}
}
bool PageDirectoryPointerTable::getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess)
{
	PageDirectoryPointerTableEntry &entry = entries[(virtualAddress >> 30) & 0x1ff];
	if (!entry.isPresent())
		return false;
	if (!entry.isPageBig())
		return entry.getTable()->getPhysicalAddress(virtualAddress, physicalAddress, isUserAccess);
	physicalAddress = entry.getAddress(PageEntry::addressMask_1gb) | (virtualAddress & (bytesPerEntry - 1));
	return true;
}

bool PageMapLevel4::mapRegion(void *pageSpace, dword &pageAllocationMap, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	word startEntry, endEntry;
	getEntryBounds(virtualAddress, len, startEntry, endEntry, 39);

	for (word i = startEntry; i <= endEntry; i++)
	{
		PageMapLevel4Entry &entry = entries[i];

		const qword virtualAddress_entryBase = virtualAddress & ~(bytesPerEntry - 1);

		if (
			virtualAddress != virtualAddress_entryBase || // first entry, partially covered
			len < bytesPerEntry							  // last entry, partially covered
		)
		{
			// partial entry: first, last, or both
			if (entry.isPresent())
			{
				if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					return false;
			}
			else
			{
				PageDirectoryPointerTable *table = (PageDirectoryPointerTable *)AllocatePage(pageSpace, pageAllocationMap);
				if (table == nullptr)
					return false;
				entry.set(table);
				if (!table->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
					return false;
			}

			// increment stuff
			qword partialLen = bytesPerEntry - (virtualAddress - virtualAddress_entryBase);
			if (partialLen > len)
				partialLen = len;

			physicalAddress += partialLen;
			virtualAddress += partialLen;
			len -= partialLen;
		}
		else
		{
			// middle, impartial entry
			if (!entry.isPresent())
			{
				PageDirectoryPointerTable *table = (PageDirectoryPointerTable *)AllocatePage(pageSpace, pageAllocationMap);
				table->clearAll(pageSpace, pageAllocationMap);
				entry.set(table);
			}
			if (!entry.getTable()->mapRegion(pageSpace, pageAllocationMap, virtualAddress, physicalAddress, len, writeAccess, userPage))
				return false;
		}
	}

	return true;
}
void PageMapLevel4::clearAll(void *pageSpace, dword &pageAllocationMap)
{
	for (int i = 0; i < PageEntry::entriesPerTable; i++)
	{
		PageMapLevel4Entry &entry = entries[i];
		// recursively deallocate child tables
		if (entry.isPresent())
		{
			PageDirectoryPointerTable *table = entry.getTable();
			table->clearAll(pageSpace, pageAllocationMap);
			DeallocatePage(pageSpace, pageAllocationMap, table);
		}
		entries[i].clear();
	}
}
bool PageMapLevel4::getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess)
{
	PageMapLevel4Entry &entry = entries[(virtualAddress >> 39) & 0x1ff];
	if (!entry.isPresent())
		return false;
	return entry.getTable()->getPhysicalAddress(virtualAddress, physicalAddress, isUserAccess);
}

PageMapLevel4 *PageMapLevel4::create(void *pageSpace, dword &pageAllocationMap)
{
	return (PageMapLevel4 *)AllocatePage(pageSpace, pageAllocationMap);
}
void PageMapLevel4::setAsCurrent()
{
	setCR3(this);
}
PageMapLevel4 &PageMapLevel4::getCurrent() { return getCR3(); }