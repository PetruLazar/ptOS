#include "acpi.h"
#include "../../utils/isriostream.h"
#include "../../debug/verbose.h"
#include <string.h>

namespace ACPI
{
	static constexpr char RSDPsignature[] = "RSD PTR ";
	static constexpr uint RSDPsignatureLen = 8;

	const word* const EBDAbaseAddressPtr = (word*)0xffffffff8000040e;
	static constexpr uint maxEBDAsearch = 1024;

	const char* const mainBIOSlower = (char*)0x000E0000;
	const char* const mainBIOSupper = (char*)0x000FFFFF;

	class RSDP
	{
	public:
		const char signature[8];
		byte checksum;
		const char OEMId[6];
		byte revision;
		dword rsdtAddr;

		// revision 2.0 fields
		dword tableLength;
		qword xsdtAddr;
		byte extChecksum;
		byte reserved[3];
		
		inline byte doChecksum() { return ACPI::doChecksum((byte*)this, 20); }
		inline byte doChecksumV2() { return ACPI::doChecksum((byte*)this, 36); }
	};
	class RSDT : public GenericSDT
	{
	public:
		uint tableEntries[1];

		inline qword getEntryCount() { return (header.length - sizeof(header)) / 4; }
		inline bool checkSignature() { return GenericSDT::checkSignature("RSDT"); }
	};
	class XSDT : public GenericSDT
	{
	public:
		qword tableEntries[1];

		inline qword getEntryCount() { return (header.length - sizeof(header)) / 8; }
		inline bool checkSignature() { return GenericSDT::checkSignature("XSDT"); }
	};

	RSDP* rsdp = nullptr;

	RSDT* rsdt = nullptr;
	XSDT* xsdt = nullptr;

	void Initialize() // RSDP will come from EFI bootloader in the future
	{
		// locate ACPI
		const char* ebda = (const char*)((ull)(*EBDAbaseAddressPtr) << 4);
		if (rsdp == nullptr)
		{
			// search in EBDA
			for (const char* iterator_rsdp = ebda; iterator_rsdp < ebda + maxEBDAsearch; iterator_rsdp += 16)
			{
				rsdp = (RSDP*)iterator_rsdp;
				for (int j = 0; j < RSDPsignatureLen; j++)
				{
					if (iterator_rsdp[j] != RSDPsignature[j])
					{
						rsdp = nullptr;
						break;
					}
				}
				if (rsdp != nullptr)
					break;
			}
		}

		if (rsdp == nullptr)
		{
			// search in main BIOS area
			for (const char* iterator_rsdp = mainBIOSlower; iterator_rsdp < mainBIOSupper; iterator_rsdp += 16)
			{
				rsdp = (RSDP*)iterator_rsdp;
				for (int j = 0; j < RSDPsignatureLen; j++)
				{
					if (iterator_rsdp[j] != RSDPsignature[j])
					{
						rsdp = nullptr;
						break;
					}
				}
				if (rsdp != nullptr)
					break;
			}
		}

		if (rsdp == nullptr)
		{
			VERBOSE_LOG("ACPI RSDP not found. Skipping...\n");
			return;
		}

		// ACPI located; do some validation
		bool valid = true;

		// for now, only support version 0 directly
		if (rsdp->doChecksum() != 0) valid = false;
		if (rsdp->revision >= 2)
		{
			if (rsdp->doChecksumV2() != 0) valid = false;
		}

		if (!valid)
		{
			VERBOSE_LOG("ACPI RSDP validation failed. Skipping...\n");
			return;
		}

		// ISR::std::cout << "RSDP:\n";
		// ISR::std::cout << "\tchecksum = " << std::ostream::base::hex << rsdp->checksum << std::ostream::base::dec << '\n';
		// ISR::std::cout << "\tOEMId = \"" << std::string(rsdp->OEMId, 6).data() << "\"\n";
		// ISR::std::cout << "\trevision = " << rsdp->revision << '\n';
		// ISR::std::cout << "\trsdtAddr = " << (void*)(ull)rsdp->rsdtAddr << '\n';
		// ISR::std::cout << "\ttableLength = " << rsdp->tableLength << '\n';
		// ISR::std::cout << "\txsdtAddr = " << (void*)(ull)rsdp->xsdtAddr << '\n';
		// ISR::std::cout << "\textChecksum = " << std::ostream::base::hex << rsdp->extChecksum << std::ostream::base::dec << '\n';

		GenericSDT *sdt;
		if (rsdp->revision >= 2)
		{
			// use XSDT
			xsdt = (XSDT*)rsdp->xsdtAddr;
			sdt = xsdt;

			if (xsdt->checkSignature() == false)
				valid = false;
		}
		else
		{
			// use RSDT
			rsdt = (RSDT*)(qword)rsdp->rsdtAddr;
			sdt = rsdt;

			if (rsdt->checkSignature() == false)
				valid = false;
		}

		// validate root table
		if (!valid || sdt->doChecksum() != 0)
		{
			VERBOSE_LOG("ACPI root table validation failed. Skipping...\n");
			return;
		}

		InitializeFADT();
		InitializeDSDT();
		InitializeMADT();
		InitializeSSDT();
	}

	void listRootEntries()
	{
		if (xsdt != nullptr)
		{
			// parse XSDT
			std::cout << "XSDT content:\n";
			ull count = xsdt->getEntryCount();
			for (ull i = 0; i < count; i++)
			{
				GenericSDT* table = (GenericSDT*)(qword)xsdt->tableEntries[i];

				std::cout << "\tLocated " << std::string(table->header.signature, 4);
				if (table->doChecksum() != 0) std::cout << " (checksum failed)";
				std::cout << '\n';
			}
		}
		else if (rsdt != nullptr)
		{
			// parse RSDT
			std::cout << "RSDT content:\n";
			ull count = rsdt->getEntryCount();
			for (ull i = 0; i < count; i++)
			{
				GenericSDT* table = (GenericSDT*)(qword)rsdt->tableEntries[i];

				std::cout << "\tLocated " << std::string(table->header.signature, 4);
				if (table->doChecksum() != 0) std::cout << " (checksum failed)";
				std::cout << '\n';
			}
		}
		else
		{
			std::cout << "No root ACPI table found or validation failed.\n";
		}
	}
	GenericSDT* getTable(const char tableId[4])
	{
		GenericSDT* retVal = nullptr;
		if (xsdt != nullptr)
		{
			// parse XSDT
			ull count = xsdt->getEntryCount();
			for (ull i = 0; i < count; i++)
			{
				GenericSDT* table = (GenericSDT*)(qword)xsdt->tableEntries[i];

				if (table->checkSignature(tableId))
				{
					retVal = table;
					break;
				}
			}
		}
		else if (rsdt != nullptr)
		{
			// parse RSDT
			ull count = rsdt->getEntryCount();
			for (ull i = 0; i < count; i++)
			{
				GenericSDT* table = (GenericSDT*)(qword)rsdt->tableEntries[i];

				if (table->checkSignature(tableId))
				{
					retVal = table;
					break;
				}
			}
		}

		if (retVal != nullptr && retVal->doChecksum() != 0)
			retVal = nullptr;

		return retVal;
	}
}