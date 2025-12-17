
// tested file
#include <core/paging.cpp>

#include <iostream.h>
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

LIBC_STUB

extern "C" void main()
{
	DEFINE_TESTS
	{
		// a list of all tests in this file
		MAKETEST(AllocatePage),
		MAKETEST(DeallocatePage),
		MAKETEST(getEntryBounds),
	};

	EXECUTE_ALL_TESTS;

	TESTMODULE_END;
}