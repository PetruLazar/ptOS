#include "fadt.h"
#include <iostream.h>

namespace ACPI
{
	volatile FADT* fadt = nullptr;

	void InitializeFADT()
	{
		fadt = (FADT*)getTable(TableId::FADT);
	}
	volatile FADT* GetFADT()
	{
		return fadt;
	}
}