#include "ssdt.h"
#include "../aml/aml.h"
#include "../acpi.h"

namespace ACPI
{
	class SSDT : public GenericSDT
	{
	public:
		byte definitionBlock[0];

		inline ull GetDefinitionBlockLen()
		{
			return header.length - ((ull)&definitionBlock - (ull)this);
		}
	};

	void InitializeSSDT()
	{
		// one or more SSDTs could be found in the root table
		ull idx = 0;

		while (true)
		{
			SSDT* ssdt = (SSDT*)getTable(TableId::SSDT, idx++);

			if (ssdt == nullptr) break;

			AML::DisplayDefinitionBlock(ssdt->definitionBlock, ssdt->GetDefinitionBlockLen());
		} 
	}
}