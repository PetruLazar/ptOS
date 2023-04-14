#pragma once
#include <types.h>

namespace GDT
{
	static constexpr qword KERNEL_CS = 0x8,
						   KERNEL_DS = 0x10,
						   USER_CS = 0x18,
						   USERCOMP_CS = 0x20,
						   USER_DS = 0x28;

	class TSS
	{
	public:
		uint reserved1;
		UnalignedField<ull> rsp[3],
			reserved2,
			ist[7],
			reserved3;
		word reserved4, iopb;

		void clear()
		{
			reserved1 = 0;
			reserved2 = 0;
			reserved3 = 0;
			reserved4 = 0;
			for (int i = 0; i < 3; i++)
				rsp[i] = 0;
			for (int i = 0; i < 7; i++)
				ist[i] = 0;
			iopb = sizeof(TSS);
		}
	};

	class SegmentDescriptor
	{
	protected:
		word limitLow = 0xffff;
		word baseLow = 0;
		byte baseMid = 0;
		byte access = 0;
		byte limitHighandFlags = 0xf;
		byte baseHigh = 0;

		static constexpr byte presentBit = 1 << 7,
							  segmentTypeBit = 1 << 4,
							  executableBit = 1 << 3,
							  directionConformingBit = 1 << 2,
							  readWriteBit = 1 << 1,
							  dplMask = 0b11 << 5,
							  dbFlag = 1 << 6,
							  lFlag = 1 << 5,
							  granularityFlag = 1 << 7,
							  accessedFlag = 1;

	public:
		inline SegmentDescriptor() {}
		inline SegmentDescriptor(byte dpl, bool isCodeOrData)
		{
			setPrivilegeLevel(dpl);
			setType(isCodeOrData);
			setGranularity(true);
			setPresent(true);
		}

		inline void setLimit(dword limit)
		{
			limitLow = limit & 0xffff;
			limitHighandFlags = (limit >> 16) & 0xf;
		}
		inline void setBase(dword base)
		{
			baseLow = base & 0xffff;
			baseMid = (base >> 16) & 0xff;
			baseHigh = (base >> 24) & 0xff;
		}

		inline void setPrivilegeLevel(byte dpl) { access = access & ~dplMask | ((dpl << 5) & dplMask); }
		inline void setPresent(bool p) { p ? access |= presentBit : access &= ~presentBit; }
		inline void setType(bool isCodeOrData) { isCodeOrData ? access |= segmentTypeBit : access &= ~segmentTypeBit; }
		inline void setExecutable(bool e) { e ? access |= executableBit : access &= ~executableBit; }
		inline void setGranularity(bool g) { g ? limitHighandFlags |= granularityFlag : limitHighandFlags &= ~granularityFlag; }
		inline void setAccessed(bool a) { a ? access |= accessedFlag : access &= ~accessedFlag; }

		inline byte getPrivilegeLevel();
		inline bool getPresent() { return access & presentBit; }
		inline bool getAccessed() { return access & accessedFlag; }
	};

	class DataSegmentDescriptor : public SegmentDescriptor
	{
	public:
		inline DataSegmentDescriptor(byte dpl, bool writable = true, bool growsDown = false) : SegmentDescriptor(dpl, true)
		{
			setExecutable(false);
			setWritable(writable);
			setDirection(growsDown);
			setLongMode(false);
		}

		inline void setDirection(bool growsDown) { growsDown ? access |= directionConformingBit : access &= ~directionConformingBit; }
		inline void setWritable(bool w) { w ? access |= readWriteBit : access &= ~readWriteBit; }

		inline void setLongMode(bool lm) { lm ? limitHighandFlags |= lFlag : limitHighandFlags &= ~lFlag; }
	};
	class CodeSegmentDescriptor : public SegmentDescriptor
	{
	public:
		inline CodeSegmentDescriptor(byte dpl, bool compatibilityMode, bool readable = true, bool conforming = false) : SegmentDescriptor(dpl, true)
		{
			setExecutable(true);
			setReadable(readable);
			setConforming(conforming);
			if (compatibilityMode)
			{
				setCompMode(true);
			}
			else
				setLongMode(true);
		}

		inline void setConforming(bool c) { c ? access |= directionConformingBit : access &= ~directionConformingBit; }
		inline void setReadable(bool r) { r ? access |= readWriteBit : access &= ~readWriteBit; }

		inline void setCompMode(bool is32bit) { is32bit ? limitHighandFlags |= dbFlag : limitHighandFlags &= ~dbFlag; }
		inline void setLongMode(bool lm) { lm ? limitHighandFlags |= lFlag : limitHighandFlags &= ~lFlag; }
	};
	class SystemSegmentDescriptor
	{
	public:
		SegmentDescriptor low;
		SegmentDescriptor high;

		inline SystemSegmentDescriptor(ull base, uint limit)
		{
			low.setBase((uint)base);
			low.setLimit(limit);
			low.setPresent(true);
			low.setExecutable(true);
			low.setAccessed(true);
			// low.set
			*(uint *)&high = base >> 32;
		}
	};
	class TSSDescriptor : public SystemSegmentDescriptor
	{
	public:
		// 0x89 access
		// 0x40 flags, prob need 0x20
		inline TSSDescriptor(TSS &tss) : SystemSegmentDescriptor((ull)&tss, sizeof(tss)) {}
	};

	void Initialize();
	void testGDT();
}