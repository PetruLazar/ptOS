#include "madt.h"
#include "acpi.h"
#include <iostream.h>
using namespace std;

namespace ACPI
{
	class MADT : public GenericSDT
	{
	public:
		class EntryGeneric
		{
		public:
			enum EntryType : byte
			{
				type0 = 0,
				type1 = 1,
				type2 = 2,
				type3 = 3,
				type4 = 4,
				type5 = 5,
				type9 = 9,
			} entryType;
			byte recordLength;

			inline EntryGeneric* next() { return (EntryGeneric*)((byte*)this + recordLength); }
		};
		class EntryType0 : public EntryGeneric
		{
		public:
			byte ACPIProcessorId;
			byte APICID;
			uint flags;
		};
		class EntryType1: public EntryGeneric
		{
		public:
			byte IOAPICID;
		private:
			byte reserved;
		public:
			uint IOAPICaddres;
			uint globalSystemInterruptBase;
		};
		class EntryType2: public EntryGeneric
		{
		public:
			byte busSource;
			byte irqSource;
			uint globalSystemInterrupt;
			ushort flags;
		};
		class EntryType3: public EntryGeneric
		{
		public:
			byte nmiSource;
		private:
			byte reserved;
		public:
			ushort flags;
			UnalignedField<uint> globalSystemInterrupt;
		};
		class EntryType4: public EntryGeneric
		{
		public:
			byte ACPIProcessorID;
			UnalignedField<ushort> flags;
			byte lintnr;
		};
		class EntryType5: public EntryGeneric
		{
			ushort reserved;
		public:
			UnalignedField<ull> localAPIDAddressOverride;
		};
		class EntryType9: public EntryGeneric
		{
			ushort reserved;
		public:
			uint localX2APICID;
			uint flags;
			uint ACPIID;
		};

		uint localAPICaddress;
		uint flags; // bit 0: legacy PIC installed
		EntryGeneric entries[0];
	};

	MADT *madt = nullptr;

	void InitializeMADT()
	{
		madt = (MADT*)ACPI::getTable(TableId::MADT);

		if (madt == nullptr)
			return; // table not found on the syste
	}
	void DisplayMADT()
	{
		if (madt == nullptr)
		{
			cout << "MADT not found";
			return;
		}

		DisplayMemoryBlock((byte*)madt, madt->header.length);

		cout << "local APIC address = " << ostream::base::hex << madt->localAPICaddress;
		cout << "\nFlags = " << madt->flags << ostream::base::dec << '\n';

		MADT::EntryGeneric *entry = madt->entries;
		uint entryIdx = 0;

		while (madt->ContainsField(*entry))
		{
			cout << "Entry " << entryIdx << ":\n";

			switch (entry ->entryType)
			{
			case MADT::EntryGeneric::type0:
			{
				cout << "  Entry type = 0 (Processor local APIC)\n";
				cout << "  Entry len = " << entry->recordLength << '\n';

				MADT::EntryType0 *entrySpecialized = (MADT::EntryType0*)entry;
				cout << "  ACPI processor ID = " << entrySpecialized->ACPIProcessorId << '\n';
				cout << "  APIC ID = " << entrySpecialized->APICID << '\n';
				cout << "  Flags = " << ostream::base::hex << entrySpecialized->flags << ostream::base::dec << '\n';

				break;
			}
			case MADT::EntryGeneric::type1:
			{
				cout << "  Entry type = 1 (I/O APIC)\n";

				MADT::EntryType1 *entrySpecialized = (MADT::EntryType1*)entry;
				cout << "  I/O APIC ID = " << entrySpecialized->IOAPICID << '\n';
				cout << "  I/O APIC address = " << (void*)(ull)entrySpecialized->IOAPICaddres << '\n';
				cout << "  Global system interrupt base = " << (void*)(ull)entrySpecialized->globalSystemInterruptBase << '\n';

				break;
			}
			case MADT::EntryGeneric::type2:
			{
				cout << "  Entry type = 2 (I/O APIC interrupt source override)\n";

				MADT::EntryType2 *entrySpecialized = (MADT::EntryType2*)entry;
				cout << "  Bus source = " << entrySpecialized->busSource << '\n';
				cout << "  IRQ source = " << entrySpecialized->irqSource << '\n';
				cout << "  Global system interrupt = " << entrySpecialized->globalSystemInterrupt << '\n';
				cout << "  Flags = " << ostream::base::hex << entrySpecialized->flags << ostream::base::dec << '\n';

				break;
			}
			case MADT::EntryGeneric::type3:
			{
				cout << "  Entry type = 3 (I/O APIC non-maskable interrupt source)\n";

				MADT::EntryType3 *entrySpecialized = (MADT::EntryType3*)entry;
				cout << "  NMI source = " << entrySpecialized->nmiSource << '\n';
				cout << "  Flags = " << ostream::base::hex << entrySpecialized->flags << ostream::base::dec << '\n';
				cout << "  Global system interrupt = " << entrySpecialized->globalSystemInterrupt << '\n';

				break;
			}
			case MADT::EntryGeneric::type4:
			{
				cout << "  Entry type = 4 (Local APIC non-maskable interrupts)\n";

				MADT::EntryType4 *entrySpecialized = (MADT::EntryType4*)entry;
				cout << "  ACPI processor ID = " << entrySpecialized->ACPIProcessorID << '\n';
				cout << "  Flags = " << ostream::base::hex << entrySpecialized->flags << ostream::base::dec << '\n';
				cout << "  LINT# = " << entrySpecialized->lintnr << '\n';

				break;
			}
			case MADT::EntryGeneric::type5:
			{
				cout << "  Entry type = 5 (Local APIC address override)\n";

				MADT::EntryType5 *entrySpecialized = (MADT::EntryType5*)entry;
				cout << "  Local APIC 64-bit address override = " << (void*&)entrySpecialized->localAPIDAddressOverride << '\n';

				break;
			}
			case MADT::EntryGeneric::type9:
			{
				cout << "  Entry type = 9 (Processor local x2APIC)\n";

				MADT::EntryType9 *entrySpecialized = (MADT::EntryType9*)entry;
				cout << "  Local x2APIC ID = " << entrySpecialized->localX2APICID << '\n';
				cout << "  Flags = " << ostream::base::hex << entrySpecialized->flags << ostream::base::dec << '\n';
				cout << "  ACPI ID = " << entrySpecialized->ACPIID << '\n';

				break;
			}
			default:
			{
				cout << "  Entry type = " << entry->entryType << " (Unknown)\n";
				break;
			}
			}

			entryIdx++;
			entry = entry->next();
		}
	}
}