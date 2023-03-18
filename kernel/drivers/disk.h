#pragma once
#include "../../libc/types.h"

namespace Disk
{
	enum class accessMode : byte
	{
		chs,
		lba28,
		lba48
	};
	enum class accessDir : byte
	{
		read,
		write
	};
	enum class IDEtype : word
	{
		SATA,
		PATA,
		SATAPI,
		PATAPI,
		unknown
	};
	extern const char *deviceTypes[];

	struct Device
	{
		byte reserved, channel, drive;
		IDEtype type;
		word signature, capabilities;
		dword commandSets, size;
		char model[41];
	};
	extern Device devices[4];

	void Initialize(dword bar0, dword bar1, dword bar2, dword bar3, dword bar4, dword bar5);

	byte accessATAdrive(accessDir dir, byte drive, uint lba, byte numsects, void *buffer_);
	byte readATAPI(byte drive, uint lba, byte numsects, void *buffer_);

	void displayError(byte device, byte err);
}