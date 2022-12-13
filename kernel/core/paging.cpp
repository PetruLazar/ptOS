#include "paging.h"
#include "../utils/iostream.h"

using namespace std;

void PageMapLevel4::mapRegion(qword &freeSpace, qword virtualAddress, qword physicalAddress, qword len)
{
	len >>= 12;
	word i1 = virtualAddress >> 12 & 0x1ff,
		 i2 = virtualAddress >> 21 & 0x1ff,
		 i3 = virtualAddress >> 30 & 0x1ff,
		 i4 = virtualAddress >> 39 & 0x1ff;
	PageDirectoryPointerTable *pdpt = getOrCreate(i4, freeSpace);
	PageDirectory *pd = pdpt->getOrCreate(i3, freeSpace);
	PageTable *pt = pd->getOrCreate(i2, freeSpace);

	while (len)
	{
		pt->entries[i1].set(physicalAddress);
		if (++i1 == 512)
		{
			i1 = 0;
			if (++i2 == 512)
			{
				i2 = 0;
				if (++i3 == 512)
				{
					i3 = 0;
					if (++i4 == 512) // this should never happen
					{
						cout << "Virtual space overflow!";
						// halt
						while (true)
							;
					}
					pdpt = getOrCreate(i4, freeSpace);
				}
				pd = pdpt->getOrCreate(i3, freeSpace);
			}
			pt = pd->getOrCreate(i2, freeSpace);
		}

		len--;
		physicalAddress += 0x1000;
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