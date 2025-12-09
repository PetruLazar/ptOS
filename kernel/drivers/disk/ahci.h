#pragma once
#include "../pci.h"
#include "../../cpu/interrupt/irq.h"

namespace AHCI
{
	void Initialize();
	void CleanUp();

	void ControllerDetected(PCI::PCIDevice &device);

	void Syscall(registers_t &regs);
}