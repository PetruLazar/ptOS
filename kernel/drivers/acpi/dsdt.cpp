#include "dsdt.h"
#include "fadt.h"
#include "aml.h"
#include <iostream.h>

namespace ACPI
{
	class DSDT : public GenericSDT
	{
	public:
		byte definitionBlock[1];

		inline ull GetDefinitionBlockLen()
		{
			return header.length - ((ull)&definitionBlock - (ull)this);
		}
	};

	DSDT* dsdt = nullptr;

	void InitializeDSDT()
	{
		// DSDT is taken from FADT, but it could also be pointed to directly by the RSDT
		volatile FADT* fadt = GetFADT();
		if (fadt != nullptr)
		{
			if (fadt->ContainsField(fadt->x_dsdt) && (void*&)fadt->x_dsdt != nullptr)
			{
				dsdt = (DSDT*)(void*&)fadt->x_dsdt;
			}
			else 
			{
				dsdt = (DSDT*)(ull)fadt->dsdt;
			}

			AML::LoadDefinitionBlock(dsdt->definitionBlock, dsdt->GetDefinitionBlockLen());
		}
	}
	void DisplayDSDT()
	{
		if (dsdt == nullptr) return;

		AML::DisplayDefinitionBlock(dsdt->definitionBlock, dsdt->GetDefinitionBlockLen());
	}
}