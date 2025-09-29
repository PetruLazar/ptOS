#include "mem.h"
#include <iostream.h>
#include "paging.h"
#include "../cpu/idt.h"
#include "sys.h"

using namespace std;

namespace Memory
{
	enum class RegionType : uint32_t
	{
		usable = 1,
		reserved,
		acpiReclaimable,
		acpiNvs,
		bad,

		limit
	};

	const char *typeToString(Memory::RegionType type)
	{
		switch (type)
		{
		case Memory::RegionType::usable:
			return "usable";
		case Memory::RegionType::reserved:
			return "reserved";
		case Memory::RegionType::acpiReclaimable:
			return "ACPI reclaimable";
		case Memory::RegionType::acpiNvs:
			return "ACPI NVS";
		default:
			return "bad";
		}
	}

	byte mapLength, mapEntrySize;
	class MapEntry
	{
	public:
		byte *base_address;
		qword length;
		RegionType type;
		uint acpiExtendableAttributes;
	} *memoryMap;

	void swapEntries(byte i, byte j)
	{
		MapEntry temp = memoryMap[i];
		memoryMap[i] = memoryMap[j];
		memoryMap[j] = temp;
	}
	void removeEntry(byte i)
	{
		mapLength--;
		for (byte k = i; k < mapLength; k++)
			memoryMap[k] = memoryMap[k + 1];
	}
	void insertEntry(byte i, byte *start, qword len, RegionType type, uint acpiExtendableAttributes)
	{
		for (byte k = mapLength; k > i; k--)
		{
			memoryMap[k] = memoryMap[k - 1];
		}
		mapLength++;
		memoryMap[i].base_address = start;
		memoryMap[i].length = len;
		memoryMap[i].type = type;
		memoryMap[i].acpiExtendableAttributes = acpiExtendableAttributes;
	}

	void sortMap()
	{
		// sort memory regions
		for (byte l = mapLength - 1; l > 0; l--)
		{
			for (byte i = 0; i < l; i++)
			{
				if (memoryMap[i].base_address > memoryMap[i + 1].base_address)
					swapEntries(i, i + 1);
			}
		}

		// mark unrecognized types as reserved
		for (byte i = 0; i < mapLength; i++)
			if (memoryMap[i].type >= RegionType::limit)
				memoryMap[i].type = RegionType::reserved;

		// merge adjacent regions of the same type
		// mark overlapping regions as the most restrictive type
		// mark unlisted regions as reserved
		for (byte i = 0; i < mapLength - 1; i++)
		{
			MapEntry &m1 = memoryMap[i],
					 &m2 = memoryMap[i + 1];
			if (m1.base_address + m1.length == m2.base_address)
			{
				// no gap between the regions
				if (m1.type == m2.type)
				{
					m1.length += m2.length;
					removeEntry(i + 1);
					i--;
				}
			}
			else if (m1.base_address + m1.length > m2.base_address)
			{
				// regions are overlapping
				if (m1.type == m2.type)
				{
					m1.length = m2.base_address - m1.base_address + m2.length;
					removeEntry(i + 1);
					i--;
				}
				else
				{
					RegionType order[] = {
						RegionType::bad,
						RegionType::reserved,
						RegionType::acpiNvs,
						RegionType::acpiReclaimable,
						RegionType::usable,
					};
					for (byte i = 0; i < 5; i++)
					{
						RegionType currType = order[i];
						if (m1.type == currType || m2.type == currType)
						{
							qword gapLen = m1.length - (m2.base_address - m1.base_address);
							if (m1.type == currType)
							{
								// overlap belongs to m1, shrink m2
								m2.length -= gapLen;
								m2.base_address += gapLen;
							}
							else
							{
								// overlap belongs to m2, shrink m1
								m1.length -= gapLen;
							}
						}
					}
					i--;
				}
			}
			else
			{
				// gap between regions
				insertEntry(i + 1, m1.base_address + m1.length, m2.base_address - m1.base_address - m1.length, RegionType::reserved, 0);
				i--;
			}
		}

		// maybe page align them?

		// page map them
	}

	void DisplayMap()
	{
		cout << "Memory map: (" << mapLength << " x " << mapEntrySize << " bytes): ";
		for (byte i = 0; i < mapLength; i++)
		{
			MapEntry &en = memoryMap[i];
			cout << "\n[" << i << "]  Base address: " << (void *)en.base_address
				 << "\n     Length: " << (void *)en.length << " (" << en.length
				 << ")\n     End address: " << (void *)(en.base_address + en.length - 1)
				 << "\n     Region type: " << typeToString(en.type) << " (" << (uint)en.type
				 << ")\n     Reserved: " << en.acpiExtendableAttributes;
			// IDT::waitForIrq1();
		}
		cout << '\n';
	}
	string getStringMemoryMap()
	{
		string ret;
		char buffer[32];
		ret = "Memory map: (";
		ulltos(buffer, mapLength);
		ret += buffer;
		ret += " x ";
		ulltos(buffer, mapEntrySize);
		ret += buffer;
		ret += " bytes): ";
		for (byte i = 0; i < mapLength; i++)
		{
			MapEntry &en = memoryMap[i];
			ret += "\n[";
			ulltos(buffer, i);
			ret += buffer;
			ret += "]  Base address: ";
			qwordToHexString(buffer, (ull)en.base_address);
			ret += buffer;
			ret += "\n     Length: ";
			qwordToHexString(buffer, (ull)en.length);
			ret += buffer;
			ret += " (";
			ulltos(buffer, en.length);
			ret += buffer;
			ret += ")\n     End address: ";
			qwordToHexString(buffer, (ull)(en.base_address + en.length - 1));
			ret += buffer;
			ret += "\n     Region type: ";
			ret += typeToString(en.type);
			ret += " (";
			ulltos(buffer, (ull)en.type);
			ret += buffer;
			ret += ")\n     Reserved: ";
			ulltos(buffer, en.acpiExtendableAttributes);
			ret += buffer;
			// IDT::waitForIrq1();
		}
		return ret;
	}
	void Initialize(byte *kernelPhysicalAddress, byte *mapEntryDescriptor, byte *mapEntries)
	{
		mapLength = *mapEntryDescriptor;
		mapEntrySize = *(mapEntryDescriptor + 1);
		memoryMap = (MapEntry *)mapEntries;

		// return;
		sortMap();

		int i = 0;
		for (i = 0; i < mapLength; i++)
			if (memoryMap[i].type == RegionType::usable && memoryMap[i].base_address != 0x0)
				break;
		MapEntry &entry = memoryMap[i];

		PageMapLevel4 *pml4 = (PageMapLevel4 *)entry.base_address;
		qword nextFree = (qword)pml4 + 0x1000;
		pml4->clearAll();

		// identity map the first 2MB of ram
		pml4->mapRegion(nextFree, 0x1000, 0x1000, 0x200000 - 0x1000, true, false);

		// map gdt, idt, 64-bit TSS and kernel stack and kernel image
		pml4->mapRegion(nextFree, 0xFFFFFFFF80000000, 0x0000, 0x80000, true, false);

		// apply virtual map
		pml4->setAsCurrent();

		// identity map the rest of RAM
		MapEntry &last = memoryMap[mapLength - 1];
		sqword leftToMap = (sqword)last.base_address + last.length - 0x200000;
		if (leftToMap > 0)
			pml4->mapRegion(nextFree, 0x200000, 0x200000, leftToMap, true, false);

		qword diff = nextFree - (qword)entry.base_address;
		entry.base_address += diff;
		entry.length -= diff;

		selectedHeap = Heap::build(entry.base_address, entry.length);
		// Heap::selectHeap(Heap::build(entry.base_address + diff, entry.length - diff));

		byte *interruptStack = (byte *)Memory::Allocate(0x10000, 0x1000);

		// map interrupt stack
		pml4->mapRegion(nextFree, 0xFFFFFFFF80080000, (ull)interruptStack, 0x10000, true, false);
	}
}