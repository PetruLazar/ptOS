#pragma once
#include "acpibase.h"
#include "fadt.h"
#include "dsdt.h"
#include "madt.h"
#include "ssdt.h"

namespace ACPI
{
	void Initialize();

	void listRootEntries();
	GenericSDT* getTable(const char tableId[4]);
}