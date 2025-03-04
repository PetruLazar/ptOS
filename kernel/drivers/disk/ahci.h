#pragma once
#include "../pci.h"

namespace AHCI
{
	void Initialize();
	void CleanUp();

	void ControllerDetected(PCI::PCIDevice &device);
}