#pragma once
#include <types.h>
#include <string.h>
#include "../pci.h"

namespace IDE
{
	enum class accessMode : byte
	{
		chs,
		lba28,
		lba48
	};
	enum class IDEtype : word
	{
		SATA,
		PATA,
		SATAPI,
		PATAPI,
		unknown
	};

	// extern const char *deviceTypes[];

	void Initialize();
	void CleanUp();
	void ControllerDetected(PCI::PCILocation location, PCI::DeviceHeader *header);
}