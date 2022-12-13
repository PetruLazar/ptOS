#pragma once
#include "../utils/types.h"

namespace GDT
{
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
							  granularityFlag = 1 << 7;

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

		inline byte getPrivilegeLevel();
		inline bool getPresent() { return access & presentBit; }
	};

	class DataSegmentDescriptor : public SegmentDescriptor
	{
	public:
		inline DataSegmentDescriptor(byte dpl, bool writable = true, bool growsDown = false) : SegmentDescriptor(dpl, true)
		{
			setExecutable(false);
			setWritable(writable);
			setDirection(growsDown);
		}

		inline void setDirection(bool growsDown) { growsDown ? access |= directionConformingBit : access &= ~directionConformingBit; }
		inline void setWritable(bool w) { w ? access |= readWriteBit : access &= ~readWriteBit; }
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
	/*class SystemSegmentDescriptor : public SegmentDescriptor
	{
	};*/

	void Initialize();
	void testGDT();
}