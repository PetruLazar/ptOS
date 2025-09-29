#pragma once
#include "../pci.h"
#include "../../cpu/idt.h"

namespace AHCI
{
	void Initialize();
	void CleanUp();

	void ControllerDetected(PCI::PCIDevice &device);

	void Syscall(registers_t &regs);
}