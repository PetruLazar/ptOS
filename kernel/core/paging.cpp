#include "paging.h"
#include <iostream.h>

using namespace std;

extern "C" void setCR3(PageMapLevel4 *pml4_ptr);
extern "C" PageMapLevel4 &getCR3();

// in pt: 4kb pages (0x1000)
// in pd: 2mb pages (0x)
// in pdpt: 1gb pages (0x)

void PageMapLevel4::mapRegion(qword &freeSpace, qword virtualAddress, qword physicalAddress, qword len, bool writeAccess, bool userPage)
{
	len >>= 12;
	word i1 = virtualAddress >> 12 & 0x1ff, // get index in pt
		i2 = virtualAddress >> 21 & 0x1ff,	// get index in pd
		i3 = virtualAddress >> 30 & 0x1ff,	// get index in pdpt
		i4 = virtualAddress >> 39 & 0x1ff;	// get index in pml4

	// PageDirectoryPointerTable *pdpt = getOrCreate(i4, freeSpace); // get pdpt address from pml4
	// PageDirectory *pd = pdpt->getOrCreate(i3, freeSpace);		  // get pd address from pdpt
	// PageTable *pt = pd->getOrCreate(i2, freeSpace);				  // get pt address from pd

	while (len)
	{
		// iterate through pml4 using i4
		PageDirectoryPointerTable *pdpt = getOrCreate(i4, freeSpace, writeAccess, userPage); // get pdpt address from pml4
		while (len)
		{
			// iterare through pdpt using i3
			if (len >= 0x40000 && ((i1 | i2) == 0) && ((physicalAddress & 0x3fffffff) == 0)) // if at least 1gb left to map, virtual address is aligned to 1gb and the physical address does not use bit 12
			{
				// use a 1gb page
				PageDirectoryPointerTableEntry &entry = pdpt->entries[i3];
				entry.set(physicalAddress, writeAccess, userPage); // PAT bit is 0
				entry.setPageSize(true);
				len -= 0x40000;
				physicalAddress += 0x40000000;
			}
			else
			{
				// cannot use 1gb page
				// get pdpt entry
				PageDirectory *pd = pdpt->getOrCreate(i3, freeSpace, writeAccess, userPage);
				while (len)
				{
					// iterate through pd using i2
					if (len >= 0x200 && (i1 == 0) && ((physicalAddress & 0x1fffff) == 0))
					{
						// use a 2mb page
						PageDirectoryEntry &entry = pd->entries[i2];
						entry.set(physicalAddress, writeAccess, userPage); // PAT bit is 0
						entry.setPageSize(true);
						len -= 0x200;
						physicalAddress += 0x200000;
					}
					else
					{
						// cannot use 2mb page
						// get pt entry
						PageTable *pt = pd->getOrCreate(i2, freeSpace, writeAccess, userPage);
						while (len)
						{
							// iterate through pt using i1
							pt->entries[i1].set(physicalAddress, writeAccess, userPage);
							len--;
							physicalAddress += 0x1000;
							if (++i1 == 512)
							{
								i1 = 0;
								break;
							}
						}
					}
					if (++i2 == 512)
					{
						i2 = 0;
						break;
					}
				}
			}
			if (++i3 == 512)
			{
				// pdpt over
				i3 = 0;
				break;
			}
		}
		if (++i4 == 512) // go to the next pml4 entry
		{
			// pml4 over, virtual space overflow
			return;
		}
	}
}
bool PageMapLevel4::getPhysicalAddress(qword virtualAddress, qword &physicalAddress, bool isUserAccess)
{
	word i1 = virtualAddress >> 12 & 0x1ff, // get index in pt
		i2 = virtualAddress >> 21 & 0x1ff,	// get index in pd
		i3 = virtualAddress >> 30 & 0x1ff,	// get index in pdpt
		i4 = virtualAddress >> 39 & 0x1ff;	// get index in pml4

	PageTable *pt;
	PageDirectory *pd;
	PageDirectoryPointerTable *pdpt;
	{
		PageMapLevel4Entry &e = entries[i4];
		if (!e.isPresent() || (isUserAccess && !e.isUserPage()))
			return false;
		pdpt = e.getTable();
	}
	{
		PageDirectoryPointerTableEntry &e = pdpt->entries[i3];
		if (!e.isPresent() || (isUserAccess && !e.isUserPage()))
			return false;
		if (e.isPageBig())
		{
			physicalAddress = e.getAddress() | (virtualAddress & 0x3fffffff);
			return true;
		}
		pd = e.getTable();
	}
	{
		PageDirectoryEntry &e = pd->entries[i2];
		if (!e.isPresent() || (isUserAccess && !e.isUserPage()))
			return false;
		if (e.isPageBig())
		{
			physicalAddress = e.getAddress() | (virtualAddress & 0x1fffff);
			return true;
		}
		pt = e.getTable();
	}
	{
		PageTableEntry &e = pt->entries[i1];
		if (!e.isPresent() || (isUserAccess && !e.isUserPage()))
			return false;
		physicalAddress = e.getAddress() | (virtualAddress & 0xfff);
		return true;
	}
}

/*void PageMapLevel4::unmapRegion(qword virtualAddress, qword len)
{
	len >>= 12;
	word i1 = virtualAddress >> 12 & 0x1ff,
		 i2 = virtualAddress >> 21 & 0x1ff,
		 i3 = virtualAddress >> 30 & 0x1ff,
		 i4 = virtualAddress >> 39 & 0x1ff;
	PageDirectoryPointerTable *pdpt;
	PageDirectory *pd;
	PageTable *pt;

	while (len)
	{

		len--;
	}
}*/

void PageMapLevel4::setAsCurrent()
{
	setCR3(this);
}
PageMapLevel4 &PageMapLevel4::getCurrent() { return getCR3(); }