// include tested file
#include <core/paging.cpp>

// include libs
#include <mem.h>
#include <test.h>

static constexpr qword heapStart = 0x7f0000000000;

TEST(AllocatePage)
{
	TEST_INIT;

	DEFINE_TESTCASES
	(
		INPUTLIST
		{
			dword map;
		},
		OUTPUTLIST
		{
			dword map;
			qword allocatedOffset;
		}
	)
	TESTCASELIST
	{
		/* Test case 0 */ {{ 0xffffff00 }, { 0xffffff01, 0x00000 }},
		/* Test case 1 */ {{ 0xffffff03 }, { 0xffffff07, 0x02000 }},
		/* Test case 2 */ {{ 0xffffff13 }, { 0xffffff17, 0x02000 }},
		/* Test case 3 */ {{ 0xffffff7f }, { 0xffffffff, 0x07000 }},
		/* Test case 4 */ {{ 0xfffffff7 }, { 0xffffffff, 0x03000 }},
		/* Test case 5 */ {{ 0xffffffff }, { 0xffffffff, 0xfffff }},
	};

	ull *pageSpace = (ull *)heapStart;

	FOREACH_TESTCASE
	{
		dword pageAllocationMap = INPUT(map);
		for (int i = 0; i < 0x1000; i++)
			pageSpace[i] = (ull)(-1);

		qword *allocatedPage = (qword *)AllocatePage(pageSpace, pageAllocationMap);

		test_assert_expected(pageAllocationMap, ==, OUTPUT(map));
		if (INPUT(map) != OUTPUT(map))
		{
			test_assert_expected_named(allocatedPage, !=, nullptr);
			if (allocatedPage == nullptr)
				continue;
			qword allocatedPageOffset = (byte *)allocatedPage - (byte *)pageSpace;
			test_assert_expected(allocatedPageOffset, ==, OUTPUT(allocatedOffset));
			for (int i = 0; i < 0x200; i++)
			{
				if (test_failed)
					break;
				test_assert_expected(allocatedPage[i], ==, 0);
			}
		}
		else
		{
			test_assert_expected_named(allocatedPage, ==, nullptr);
		}
	}

	TEST_END;
}
TEST(DeallocatePage)
{
	TEST_INIT;

	DEFINE_TESTCASES
	(
		INPUTLIST
		{
			dword map;
			qword tableIdx;
		},
		OUTPUTLIST
		{
			dword map;
		}
	)
	TESTCASELIST
	{
		/* Test case 0*/ {{ 0xffffffff,  0 }, { 0xfffffffe }},
		/* Test case 1*/ {{ 0xffffffff, 31 }, { 0x7fffffff }},
		/* Test case 2*/ {{ 0xffffffff, 10 }, { 0xfffffbff }},
		/* Test case 3*/ {{ 0x00003fff, 13 }, { 0x00001fff }},
		/* Test case 4*/ {{ 0x00000001,  0 }, { 0x00000000 }},
		/* Test case 5*/ {{ 0x00008000, 15 }, { 0x00000000 }},
	};

	byte *pageSpace = (byte *)heapStart;

	FOREACH_TESTCASE
	{
		dword pageAllocationMap = INPUT(map);
		void *table = pageSpace + 0x1000 * INPUT(tableIdx);

		DeallocatePage(pageSpace, pageAllocationMap, table);

		test_assert_expected(pageAllocationMap, ==, OUTPUT(map));
	}

	TEST_END;
}
TEST(getEntryBounds)
{
	TEST_INIT;

	DEFINE_TESTCASES
	(
		INPUTLIST
		{
			qword virtualAddress;
			qword len;
			int bitShift;
		},
		OUTPUTLIST
		{
			word startEntry;
			word endEntry;
		}
	)
	TESTCASELIST
	{
		// entry size: 0x1000
		/* Test case  0 */ {{ 0x5000, 0x1000, 12 }, { 0x5, 0x5 }},
		/* Test case  1 */ {{ 0x205000, 0x2000, 12 }, { 0x5, 0x6 }},
		/* Test case  2 */ {{ 0x1f0000, 0x30000, 12 }, { 0x1f0, 0x1ff }},
		/* Test case  3 */ {{ 0x200000, 0x40000000, 12 }, { 0x0, 0x1ff }},

		// entry size: 0x200000
		/* Test case  4 */ {{ 0x40400000, 0x80000000, 21 }, { 0x2, 0x1ff }},
		/* Test case  5 */ {{ 0x40445000, 0x00005000, 21 }, { 0x2, 0x2 }},
		/* Test case  6 */ {{ 0x40545000, 0x000bb000, 21 }, { 0x2, 0x2 }},
		/* Test case  7 */ {{ 0x40745000, 0x00105000, 21 }, { 0x3, 0x4 }},

		// entry size: 0x40000000
		/* Test case  8 */ {{ 0x40745000, 0x00105000, 30 }, { 0x1, 0x1 }},
		/* Test case  9 */ {{ 0x3ffff000, 0x00001000, 30 }, { 0x0, 0x0 }},
		/* Test case 10 */ {{ 0x3ffff000, 0x00002000, 30 }, { 0x0, 0x1 }},
		/* Test case 11 */ {{ 0x3ffff000, 0x40002000, 30 }, { 0x0, 0x2 }},
		/* Test case 12 */ {{ 0x40000000, 0x40000000, 30 }, { 0x1, 0x1 }},
		/* Test case 13 */ {{ 0x30000000, 0x40000000, 30 }, { 0x0, 0x1 }},
		/* Test case 14 */ {{ 0x30000000, 0x50000000, 30 }, { 0x0, 0x1 }},
		/* Test case 15 */ {{ 0x4900000000, 0x4000000000, 30 }, { 0x124, 0x1ff }},

		// entry size: 0x80 0000 0000
		/* Test case 16 */ {{ 0x008100000000, 0x000040000000, 39 }, { 0x1, 0x1 }},
		/* Test case 17 */ {{ 0x008000000000, 0x008000000000, 39 }, { 0x1, 0x1 }},
		/* Test case 18 */ {{ 0x008000000000, 0x038000000000, 39 }, { 0x1, 0x7 }},
		/* Test case 19 */ {{ 0x008c00000000, 0x039000000000, 39 }, { 0x1, 0x8 }},
	};

	FOREACH_TESTCASE
	{
		qword virtualAdddress = INPUT(virtualAddress);
		qword len = INPUT(len);
		int bitShift = INPUT(bitShift);

		word startEntry, endEntry;

		getEntryBounds(virtualAdddress, len, startEntry, endEntry, bitShift);

		test_assert_expected(startEntry, ==, OUTPUT(startEntry));
		test_assert_expected(endEntry, ==, OUTPUT(endEntry));
	}

	TEST_END;
}
TEST(getPhysicalAddress)
{
	TEST_INIT;

	DEFINE_TESTCASES
	(
		INPUTLIST
		{
			word pml4Index;
			word pdptIndex;
			word pdIndex;
			word ptIndex;
			word pageOffset;

			byte pageFrameAtLevel; // 1-pte    2-pde    3-pdpte

			ull pageFrame;
			PageEntry::EntryAttributes attributes;

			bool pml4SupressPresentBit;
			bool pdptSupressPresentBit;
			bool pdSupressPresentBit;
			bool ptSupressPresentBit;

			bool accessAsUser;
		},
		OUTPUTLIST
		{
			bool virtualAddressHasMapping;
		}
	)
	TESTCASELIST
	{
		/* Test case  0 */ {{ 0x000, 0x000, 0x001, 0x000,   0x178, 0x1, 0x0000010fe8000, PageEntry::EntryAttributes(PageEntry::userPageBit), 0, 0, 0, 0, 1 }, { 1 }}, // mapping for pt, user, access as user
		/* Test case  1 */ {{ 0x000, 0x023, 0x0bc, 0x0b2,   0x061, 0x1, 0x00000c8423000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 1 }, { 0 }}, // mapping for pt, supervisor, access as user
		/* Test case  2 */ {{ 0x0b8, 0x000, 0x000, 0x10b,   0x8d0, 0x1, 0x0000025adf000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 0 }, { 1 }}, // mapping for pt, supervisor, access as supervisor
		/* Test case  3 */ {{ 0x105, 0x100, 0x1f0, 0x1c6,   0x9fa, 0x1, 0x000288477c000, PageEntry::EntryAttributes(),                       0, 0, 0, 1, 0 }, { 0 }}, // mapping for pt, supervisor, access as supervisor, but pte not present
		/* Test case  4 */ {{ 0x1ff, 0x1ff, 0x16e, 0x0f4,   0x34b, 0x1, 0x00000484de000, PageEntry::EntryAttributes(),                       1, 0, 0, 0, 0 }, { 0 }}, // mapping for pt, supervisor, access as supervisor, but pml4e not present

		/* Test case  5 */ {{ 0x01b, 0x132, 0x04b, 0x049,   0x340, 0x2, 0x00f8462c49000, PageEntry::EntryAttributes(PageEntry::userPageBit), 0, 0, 0, 0, 1 }, { 1 }}, // mapping for pd, user, access as user
		/* Test case  6 */ {{ 0x05f, 0x1a5, 0x0ec, 0x1f1,   0xcfa, 0x2, 0x00000001f1000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 1 }, { 0 }}, // mapping for pd, supervisor, access as user
		/* Test case  7 */ {{ 0x0e4, 0x042, 0x01b, 0x0f1,   0x7da, 0x2, 0x003d2e48f1000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 0 }, { 1 }}, // mapping for pd, supervisor, access as supervisor
		/* Test case  8 */ {{ 0x175, 0x0dc, 0x10e, 0x1bb,   0x459, 0x2, 0x000008b3bb000, PageEntry::EntryAttributes(),                       0, 0, 1, 0, 0 }, { 0 }}, // mapping for pd, supervisor, access as supervisor, but pde not present
		/* Test case  9 */ {{ 0x1bd, 0x000, 0x1ff, 0x0e9,   0x8d0, 0x2, 0x0000025ae9000, PageEntry::EntryAttributes(),                       0, 1, 0, 0, 0 }, { 0 }}, // mapping for pd, supervisor, access as supervisor, but pdpte not present

		/* Test case 10 */ {{ 0x000, 0x18d, 0x029, 0x16c,   0xc62, 0x3, 0x00aef0536c000, PageEntry::EntryAttributes(PageEntry::userPageBit), 0, 0, 0, 0, 1 }, { 1 }}, // mapping for pdpt, user, access as user
		/* Test case 11 */ {{ 0x03d, 0x06f, 0x023, 0x0d6,   0x472, 0x3, 0x00bc5446d6000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 1 }, { 0 }}, // mapping for pdpt, supervisor, access as user
		/* Test case 12 */ {{ 0x0c4, 0x1ff, 0x1ff, 0x000,   0x4e8, 0x3, 0x005f33fe00000, PageEntry::EntryAttributes(),                       0, 0, 0, 0, 0 }, { 1 }}, // mapping for pdpt, supervisor, access as supervisor
		/* Test case 13 */ {{ 0x0cc, 0x0ad, 0x184, 0x152,   0x414, 0x3, 0x0000030952000, PageEntry::EntryAttributes(),                       0, 1, 0, 0, 0 }, { 0 }}, // mapping for pdpt, supervisor, access as supervisor, but pdpte not present
		/* Test case 14 */ {{ 0x1dc, 0x000, 0x0e1, 0x190,   0x05d, 0x3, 0x00ac59c390000, PageEntry::EntryAttributes(),                       1, 0, 0, 0, 0 }, { 0 }}, // mapping for pdpt, supervisor, access as supervisor, but pml4e not present
	};

	PageMapLevel4* pml4 = (PageMapLevel4*)(heapStart);
	PageDirectoryPointerTable* pdpt = (PageDirectoryPointerTable*)(heapStart + 0x1000);
	PageDirectory* pd = (PageDirectory*)(heapStart + 0x2000);
	PageTable* pt = (PageTable*)(heapStart + 0x3000);

	FOREACH_TESTCASE
	{
		// clear all tables
		for (word i = 0; i < 0x200; i++)
		{
			pml4->entries[i].clear();
			pdpt->entries[i].clear();
			pd->entries[i].clear();
			pt->entries[i].clear();
		}

		// set the entries according to test case inputs
		byte pageFrameAtLevel = INPUT(pageFrameAtLevel);
		word pml4Index = INPUT(pml4Index);
		word pdptIndex = INPUT(pdptIndex);
		word pdIndex = INPUT(pdIndex);
		word ptIndex = INPUT(ptIndex);
		ull pageFrame = INPUT(pageFrame);
		PageEntry::EntryAttributes attributes = INPUT(attributes);

		pml4->entries[pml4Index].set(pdpt);
		if (INPUT(pml4SupressPresentBit)) pml4->entries[pml4Index].setPresent(false);

		if (pageFrameAtLevel < 3) pdpt->entries[pdptIndex].set(pd);
		else pdpt->entries[pdptIndex].set(pageFrame, attributes);
		if (INPUT(pdptSupressPresentBit)) pdpt->entries[pdptIndex].setPresent(false);

		if (pageFrameAtLevel < 2) pd->entries[pdIndex].set(pt);
		else pd->entries[pdIndex].set(pageFrame, attributes);
		if (INPUT(pdSupressPresentBit)) pd->entries[pdIndex].setPresent(false);

		pt->entries[ptIndex].set(pageFrame, attributes);
		if (INPUT(ptSupressPresentBit)) pt->entries[ptIndex].setPresent(false);

		// prepare virtual address and normalize
		qword virtualAddress = 
			(INPUT(pageOffset) & 0xfff) |
			((qword)(ptIndex & 0x1ff) << 12) |
			((qword)(pdIndex & 0x1ff) << 21) |
			((qword)(pdptIndex & 0x1ff) << 30) |
			((qword)(pml4Index & 0x1ff) << 39);
		if (virtualAddress & ((qword)1 << 47)) virtualAddress |= 0xffff000000000000;

		// call the tested functions and
		qword physicalAddress;
		bool virtualAddressHasMapping = pml4->getPhysicalAddress(virtualAddress, physicalAddress, INPUT(accessAsUser));

		test_assert_expected(virtualAddressHasMapping, ==, OUTPUT(virtualAddressHasMapping));
		if (OUTPUT(virtualAddressHasMapping))
		{
			test_assert_expected(physicalAddress, ==, (pageFrame | INPUT(pageOffset)));
		}
	}

	TEST_END;
}
TEST(mapRegion)
{
	TEST_INIT;

	DEFINE_TESTCASES
	(
		INPUTLIST
		{
			// virtual address
			qword virtualAddress;
			qword physicalAddress;
			qword len;
			PageEntry::EntryAttributes attributes;
		},
		OUTPUTLIST
		{
			bool success;
			dword pageAllocationMap;

			const int sampleCount;
			OUTPUTARRAY(qword) sampleVirtualAddress;
			OUTPUTARRAY(qword) samplePhysicalAddress;
			OUTPUTARRAY(bool) sampleAsUser;
			OUTPUTARRAY(bool) sampleResult;
		}
	)
	TESTCASELIST
	{
		/* Test case  0 */ {{ 0xffffa92cebd66000, 0x70f9fc8e49000, 0x00000001000, PageEntry::EntryAttributes(PageEntry::userPageBit)    }, { 1, 0xffff000f,    4, ARRAYDATA(qword) { 0xffffa92cebd65f28, 0xffffa92cebd66a2b, 0xffffa92cebd66210, 0xffffa92cebd67000 }, ARRAYDATA(qword) { 0x0000000000000, 0x70f9fc8e49a2b, 0x70f9fc8e49210, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0 }, ARRAYDATA(bool) { 0, 1, 1, 0 }}}, // map a 4kb page
		/* Test case  1 */ {{ 0xfffffc4c5f5d6000, 0x7609d8bcad000, 0x00000048000, PageEntry::EntryAttributes()                          }, { 1, 0xffff001f,    7, ARRAYDATA(qword) { 0xfffffc4c5f5d5fc2, 0xfffffc4c5f5d6004, 0xfffffc4c5f5fd0c7, 0xfffffc4c5f5fe0a3, 0xfffffc4c5f60ec04, 0xfffffc4c5f61db73, 0xfffffc4c5f61e21c }, ARRAYDATA(qword) { 0x0000000000000, 0x7609d8bcad004, 0x0000000000000, 0x7609d8bcd50a3, 0x0000000000000, 0x7609d8bcf4b73, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 1, 0, 0 }, ARRAYDATA(bool) { 0, 1, 0, 1, 0, 1, 0 }}}, // map multiple 4kb pages crossing pd entries
		/* Test case  2 */ {{ 0x0000450651e00000, 0xffa4a921c1000, 0x00000200000, PageEntry::EntryAttributes(PageEntry::writeAccessBit) }, { 1, 0xffff000f,    6, ARRAYDATA(qword) { 0x0000450651dfffff, 0x0000450651e012fe, 0x0000450651e012fe, 0x0000450651ff487c, 0x0000450651ff487c, 0x0000450652000000 }, ARRAYDATA(qword) { 0x0000000000000, 0xffa4a921c22fe, 0x0000000000000, 0xffa4a923b587c, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 1, 0 }, ARRAYDATA(bool) { 0, 1, 0, 1, 0, 0 }}}, // map a 2mb range, virtual address aligned, but physical address not aligned
		/* Test case  3 */ {{ 0x0000055365400000, 0x175838161e000, 0x00001400000, PageEntry::EntryAttributes(PageEntry::userPageBit)    }, { 1, 0xffff1fff,    6, ARRAYDATA(qword) { 0x00000553653fff88, 0x000005536540024b, 0x0000055365f21c9d, 0x0000055365f21c9d, 0x00000553667ff6d8, 0x0000055366800000 }, ARRAYDATA(qword) { 0x0000000000000, 0x175838161e24b, 0x175838213fc9d, 0x175838213fc9d, 0x1758382a1d6d8, 0x0000000000000 }, ARRAYDATA(bool) { 0, 1, 1, 0, 0, 1 }, ARRAYDATA(bool) { 0, 1, 1, 1, 1, 0 }}}, // map a n*2mb range, virtual address aligned, but physical address not aligned
		/* Test case  4 */ {{ 0x0000055365400000, 0x175838161e000, 0x00007a00000, PageEntry::EntryAttributes(PageEntry::userPageBit)    }, { 0, 0xffffffff,    0, nullptr, nullptr, nullptr, nullptr }}, // map a n*2mb range, virtual address aligned, but physical address not aligned, too many to fit in the allocation map

		/* Test case  5 */ {{ 0x000022a716400000, 0x248b7e4c00000, 0x00000200000, PageEntry::EntryAttributes(PageEntry::pageWriteThroughBit | PageEntry::pageCacheDisable) }, { 1, 0xffff0007,    6, ARRAYDATA(qword) { 0x000022a7163ffff8, 0x000022a716400000, 0x000022a716400000, 0x000022a7165ff518, 0x000022a7165ff518, 0x000022a716600514 }, ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x248b7e4c00000, 0x0000000000000, 0x248b7e4dff518, 0x0000000000000 }, ARRAYDATA(bool) { 0, 1, 0, 1, 0, 0 }, ARRAYDATA(bool) { 0, 0, 1, 0, 1, 0 }}}, // map a 2mb page
		/* Test case  6 */ {{ 0x00004a4fffc00000, 0xd5cb343600000, 0x00000a00000, PageEntry::EntryAttributes(PageEntry::userPageBit)                                       }, { 1, 0xffff000f,    7, ARRAYDATA(qword) { 0x00004a4fffbfffff, 0x00004a4fffc00000, 0x00004a5000101d5b, 0x00004a5000101d5b, 0x00004a50005fffff, 0x00004a5000600000, 0x00004a5000600000 }, ARRAYDATA(qword) { 0x0000000000000, 0xd5cb343600000, 0xd5cb343b01d5b, 0xd5cb343b01d5b, 0xd5cb343ffffff, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 1, 0, 1, 0, 0, 1 }, ARRAYDATA(bool) { 0, 1, 1, 1, 1, 0, 0 }}}, // map multiple 2mb pages crossing pdpt entries
		/* Test case  7 */ {{ 0x0000751640000000, 0x3738481e00000, 0x00040000000, PageEntry::EntryAttributes(PageEntry::pageCacheDisable)                                  }, { 1, 0xffff0007,    9, ARRAYDATA(qword) { 0x000075163fffffff, 0x0000751640000000, 0x00007516500015ab, 0x000075165ffa1b2f, 0x0000751660015cc6, 0x0000751670b1cc68, 0x000075167fffffff, 0x0000751680000000, 0x0000751680000000 }, ARRAYDATA(qword) { 0x0000000000000, 0x3738481e00000, 0x0000000000000, 0x37384a1da1b2f, 0x37384a1e15cc6, 0x0000000000000, 0x37384c1dfffff, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 0, 1, 0, 1, 0 }, ARRAYDATA(bool) { 0, 1, 0, 1, 1, 0, 1, 0, 0 }}}, // map a 1gb range, virtual address aligned, but physical address not aligned
		/* Test case  8 */ {{ 0xffffe64c00000000, 0x0dede0e400000, 0x000c0000000, PageEntry::EntryAttributes(PageEntry::writeAccessBit)                                    }, { 1, 0xffff001f,   10, ARRAYDATA(qword) { 0xffffe64bffffffff, 0xffffe64bffffffff, 0xffffe64c00000000, 0xffffe64c00000000, 0xffffe64c40000000, 0xffffe64c60000000, 0xffffe64c60000000, 0xffffe64c80000000, 0xffffe64cbfffffff, 0xffffe64cc0000000 }, ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x0dede0e400000, 0x0000000000000, 0x0dede4e400000, 0x0dede6e400000, 0x0000000000000, 0x0dede8e400000, 0x0dedece3fffff, 0x0000000000000 }, ARRAYDATA(bool) { 0, 1, 0, 1, 0, 0, 1, 0, 0, 0 }, ARRAYDATA(bool) { 0, 0, 1, 0, 1, 1, 0, 1, 1, 0 }}}, // map a n*1gb range, virtual address aligned, but physical address not aligned

		/* Test case  9 */ {{ 0xffffd08b00000000, 0xa7ca340000000, 0x00040000000, PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::pageWriteThroughBit)   }, { 1, 0xffff0003,    9, ARRAYDATA(qword) { 0xffffd08affffffff, 0xffffd08b00000000, 0xffffd08b00000000, 0xffffd08b10000000, 0xffffd08b20000000, 0xffffd08b30000000, 0xffffd08b3fffffff, 0xffffd08b3fffffff, 0xffffd08b40000000 }, ARRAYDATA(qword) { 0x0000000000000, 0xa7ca340000000, 0x0000000000000, 0xa7ca350000000, 0xa7ca360000000, 0xa7ca370000000, 0xa7ca37fffffff, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 0, 0, 0, 1, 0 }, ARRAYDATA(bool) { 0, 1, 0, 1, 1, 1, 1, 0, 0 }}}, // map a 1gb page
		/* Test case 10 */ {{ 0xffff26ff80000000, 0xa12ebc0000000, 0x000c0000000, PageEntry::EntryAttributes(PageEntry::pageWriteThroughBit | PageEntry::pageCacheDisable) }, { 1, 0xffff0007,   10, ARRAYDATA(qword) { 0xffff26ff7fffffff, 0xffff26ff80000000, 0xffff26ff80000000, 0xffff26ffa0000000, 0xffff26ffe0000000, 0xffff270000000000, 0xffff270000000000, 0xffff270020000000, 0xffff27003fffffff, 0xffff270040000000 }, ARRAYDATA(qword) { 0x0000000000000, 0xa12ebc0000000, 0x0000000000000, 0xa12ebe0000000, 0xa12ec20000000, 0xa12ec40000000, 0x0000000000000, 0xa12ec60000000, 0xa12ec7fffffff, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }, ARRAYDATA(bool) { 0, 1, 0, 1, 1, 1, 0, 1, 1, 0 }}}, // map multiple 1gb pages, crossing pml4 entries


		/* Test case 11 */ {{ 0x00006e8d405fe000, 0x472a954bfe000, 0x00000605000, PageEntry::EntryAttributes(PageEntry::userPageBit | PageEntry::writeAccessBit)         }, { 1, 0xffff001f,   13, ARRAYDATA(qword) { 0x00006e8d40400000, 0x00006e8d405fd000, 0x00006e8d405fdfff, 0x00006e8d405fe000, 0x00006e8d405ff000, 0x00006e8d40600000, 0x00006e8d40800000, 0x00006e8d40a00000, 0x00006e8d40c00000, 0x00006e8d40c01000, 0x00006e8d40c02000, 0x00006e8d40c03000, 0x00006e8d40e00000 }, ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x472a954bfe000, 0x472a954bff000, 0x472a954c00000, 0x472a954e00000, 0x472a955000000, 0x472a955200000, 0x472a955201000, 0x472a955202000, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 0, 0, 0, 1, 0, 1 ,1, 0, 1, 0, 0 }, ARRAYDATA(bool) { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 }}}, // map multiple 4kb pages and multiple 2mb pages
		/* Test case 12 */ {{ 0x000067923fffe000, 0x96ad27fffe000, 0x00080003000, PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::pageWriteThroughBit) }, { 1, 0xffff003f,   18, ARRAYDATA(qword) { 0x0000679200000000, 0x000067923fe00000, 0x000067923fe00000, 0x000067923fffd000, 0x000067923fffe000, 0x000067923fffe000, 0x000067923ffff000, 0x0000679240000000, 0x0000679240000000, 0x0000679280000000, 0x00006792c0000000, 0x00006792c0000000, 0x00006792c0001000, 0x00006792c0001000, 0x00006792c0200000, 0x00006792c0200000, 0x0000679300000000, 0x0000679300000000 }, ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x96ad27fffe000, 0x0000000000000, 0x96ad27ffff000, 0x96ad280000000, 0x0000000000000, 0x96ad2c0000000, 0x96ad300000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1 }, ARRAYDATA(bool) { 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 }}}, // map multiple 4kb pages and multiple 1gb pages
		/* Test case 13 */ {{ 0x0000057fbfa00000, 0x78da1ffa00000, 0x000c1000000, PageEntry::EntryAttributes(PageEntry::writeAccessBit)                                  }, { 1, 0xffff001f,   21, ARRAYDATA(qword) { 0x0000057f80000000, 0x0000057fbf800000, 0x0000057fbf800000, 0x0000057fbf9ff000, 0x0000057fbfa00000, 0x0000057fbfa00000, 0x0000057fbfc00000, 0x0000057fbfe00000, 0x0000057fc0000000, 0x0000058000000000, 0x0000058000000000, 0x0000058040000000, 0x0000058080000000, 0x0000058080200000, 0x0000058080400000, 0x0000058080400000, 0x0000058080600000, 0x0000058080800000, 0x0000058080a00000, 0x0000058080a00000, 0x00000580c0000000 },ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x78da1ffa00000, 0x0000000000000, 0x78da1ffc00000, 0x78da1ffe00000, 0x78da200000000, 0x78da240000000, 0x0000000000000, 0x78da280000000, 0x78da2c0000000, 0x78da2c0200000, 0x78da2c0400000, 0x0000000000000, 0x78da2c0600000, 0x78da2c0800000, 0x0000000000000, 0x0000000000000, 0x0000000000000 },ARRAYDATA(bool) { 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0 },ARRAYDATA(bool) { 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0 }}}, // map multiple 2mb pages and multiple 1gb pages (range crossing pml4 entries)

		/* Test case 14 */ {{ 0xffffc97f7ff77000, 0xb4364bff77000, 0x1018709b000, PageEntry::EntryAttributes(PageEntry::userPageBit | PageEntry::writeAccessBit        ) }, { 1, 0xffff01ff,   20, ARRAYDATA(qword) { 0xffffc90000000000, 0xffffc97f40000000, 0xffffc97f7fe00000, 0xffffc97f7ff76000, 0xffffc97f7ff77000, 0xffffc97f7ff96000, 0xffffc97f80000000, 0xffffc98000000000, 0xffffca0000000000, 0xffffca8000000000, 0xffffca8040000000, 0xffffca8100000000, 0xffffca8105400000, 0xffffca8107000000, 0xffffca810700b000, 0xffffca8107011000, 0xffffca8107012000, 0xffffca8107200000, 0xffffca8140000000, 0xffffcb0000000000 }, ARRAYDATA(qword) { 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0xb4364bff77000, 0xb4364bff96000, 0xb4364c0000000, 0xb436540000000, 0xb43e540000000, 0xb446540000000, 0xb446580000000, 0xb446640000000, 0xb446645400000, 0xb446647000000, 0xb44664700b000, 0xb446647011000, 0x0000000000000, 0x0000000000000, 0x0000000000000, 0x0000000000000 }, ARRAYDATA(bool) { 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 }, ARRAYDATA(bool) { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }}}, // map multiple 4kb pages, multiple 2mb pages, and multiple 1gb pages (crossing multiple pml4 entries, including full length entries)
	};

	FOREACH_TESTCASE
	{
		void* pageSpace = (void*)heapStart;
		dword pageAllocationMap = 0xffff0000;

		PageMapLevel4 *pml4 = PageMapLevel4::create(pageSpace, pageAllocationMap);

		test_assert_expected_named(pml4, !=, nullptr);
		if (pml4 == nullptr)
			continue;

		bool success = pml4->mapRegion(pageSpace, pageAllocationMap, INPUT(virtualAddress), INPUT(physicalAddress), INPUT(len), INPUT(attributes));

		test_assert_expected(success, ==, OUTPUT(success));
		test_assert_expected_hex(pageAllocationMap, ==, OUTPUT(pageAllocationMap));

		int count = OUTPUT(sampleCount);
		for (int sample = 0; sample < count; sample++)
		{
			qword physicalAddress;
			success = pml4->getPhysicalAddress(OUTPUT(sampleVirtualAddress)[sample], physicalAddress, OUTPUT(sampleAsUser)[sample]);

			test_assert_expected(success, ==, OUTPUT(sampleResult)[sample]);
			if (success)
			{
				test_assert_expected_hex(physicalAddress, ==, OUTPUT(samplePhysicalAddress)[sample]);
			}
			if (success != OUTPUT(sampleResult)[sample] || (success && physicalAddress != OUTPUT(samplePhysicalAddress)[sample]))
			{
				cout << "\nFAILED";
				ColorizeVerdict(false);
				cout << " at sample " << sample;
			}
		}

		// clean up
		pml4->clearAll(pageSpace, pageAllocationMap);
		test_assert_expected_hex(pageAllocationMap, ==, 0xffff0001);

		for (int sample = 0; sample < count; sample++)
		{
			qword physicalAddress;
			success = pml4->getPhysicalAddress(OUTPUT(sampleVirtualAddress)[sample], physicalAddress, OUTPUT(sampleAsUser)[sample]);

			test_assert_expected(success, ==, false);
			if (success != false)
			{
				cout << "\nFAILED";
				ColorizeVerdict(false);
				cout << " at sample " << sample << " (after cleanup)";
			}
		}
	}

	TEST_END;
}
TEST(mapRegion_expansion)
{
	cout << " Not implemented yet...";
	return false;
}
TEST(mapRegion_collapse)
{
	cout << " Not implemented yet...";
	return false;
}

#include "test_libc_stub.h"

extern "C" void main()
{
	DEFINE_TESTS
	{
		// a list of all tests in this file
		MAKETEST(AllocatePage),
		MAKETEST(DeallocatePage),
		MAKETEST(getEntryBounds),
		MAKETEST(getPhysicalAddress),
		MAKETEST(mapRegion),
		MAKETEST(mapRegion_expansion),
		MAKETEST(mapRegion_collapse),
	};

	EXECUTE_ALL_TESTS;

	TESTMODULE_END;
}