#include "disk.h"

#include "ide.h"
#include "ahci.h"
#include <math.h>

using namespace PCI;
using namespace std;

namespace Disk
{
	static constexpr byte PMBRPartitionType = 0xee;

	vector<StorageDevice *> *devices;
	char nextLetter = 'c';

	class MBRpartitionEntry
	{
	public:
		byte attributes, startHead;
		word startSecAndCyl;
		byte type, endHead;
		word endSecAndCyl;
		uint lbaStart, lbaLen;
	};
	class GPTHeader
	{
	public:
		char signature[8];
		uint gptRevision,
			headerSize,
			headerCRC32,
			reserved;
		ull headerLba,
			altHeaderLba,
			firstUsableBlock,
			lastUsableBlock;
		char diskGuid[16];
		ull partitionArrayLba;
		uint partitionCount,
			partitionEntrySize,
			partitionArrayCRC32;
	};
	class GPTEntry
	{
	public:
		char partitionTypeGuid[16],
			uniquePartitionGuid[16];
		ull startLba,
			endLba,
			attributes;

		char16_t partitionName[36];
		// char16_t *partitionNameStart() { return (char16_t *)(this + 1); }
	};

	void Initialize()
	{
		devices = new vector<StorageDevice *>;

		IDE::Initialize();
		AHCI::Initialize();
	}
	void CleanUp()
	{
		IDE::CleanUp();
		AHCI::CleanUp();

		for (auto &device : *devices)
		{
			for (auto &part : device->partitions)
				delete part;
			delete device;
		}
		delete devices;
	}
	void ControllerDetected(PCIDevice &device)
	{
		switch ((MassStorageController)device.header.subclass)
		{
		case MassStorageController::IDEController:
			IDE::ControllerDetected(device);
			break;
		case MassStorageController::SATAcontroller:
			AHCI::ControllerDetected(device);
			break;
		default:
			// unsupported device
			break;
		}
	}
	bool intersectPartition(const MBRpartitionEntry &part1, const MBRpartitionEntry &part2)
	{
		if (part1.type == 0 || part2.type == 0)
			return false;
		return part1.lbaStart < part2.lbaStart ? (part1.lbaStart + part1.lbaLen > part2.lbaStart) : (part2.lbaStart + part2.lbaLen > part1.lbaStart);
	}
	void StorageDevice::detectPartitions()
	{
		byte *bootSector = new byte[512];
		if (access(accessDir::read, 0, 1, bootSector) != result::success)
			return;
		MBRpartitionEntry *entries = (MBRpartitionEntry *)(bootSector + 0x1be);

		// check if this is a protective mbr or a true mbr
		ull partitionCount = 0;
		for (int i = 0; i < 4; i++)
			// count non-null partition entries
			if (entries[i].type)
				partitionCount++;
		if (partitionCount == 1)
		{
			// this might be a protective mbr
			for (int i = 0; i < 4; i++)
			{
				if (entries[i].type == PMBRPartitionType)
				{
					// this is a protective MBR: search GPT
					GPTHeader *gpt = (GPTHeader *)new byte[512];
					uint lbaStart = entries[i].lbaStart;
					delete[] bootSector;
					if (access(accessDir::read, lbaStart, 1, (byte *)gpt) != result::success)
						return;
					if (string(gpt->signature, 8) != "EFI PART" || lbaStart != gpt->headerLba || gpt->partitionEntrySize != sizeof(GPTEntry))
					{
						delete gpt;
						return;
					}
					ull partitionArrayLbaLen = integerCeilDivide(gpt->partitionCount * gpt->partitionEntrySize, 0x200);
					GPTEntry *gptEntries = (GPTEntry *)new byte[partitionArrayLbaLen * 0x200];
					if (access(accessDir::read, gpt->partitionArrayLba, partitionArrayLbaLen, (byte *)gptEntries) != result::success)
					{
						delete gpt;
						return;
					}
					for (int p = 0; p < gpt->partitionCount; p++)
					{
						bool unused = true;
						for (char c : gptEntries[p].partitionTypeGuid)
							if (c)
							{
								unused = false;
								break;
							}
						if (unused) // or if attributes & 1, firmware required, or if attributes & 2, required by OS to boot
							continue;
						Partition *part = new Partition();
						part->disk = this;
						part->lbaStart = gptEntries[p].startLba;
						part->lbaLen = gptEntries[p].endLba - gptEntries[p].startLba + 1;
						part->letter = nextLetter++;
						partitions.push_back(part);
					}
					delete gpt;
					delete[] gptEntries;
					return;
				}
				if (entries[i].type)
					break;
			}
		}
		// this is not a protective mbr
		for (int i = 0; i < 4; i++)
			if (entries[i].type)
			{
				Partition *part = new Partition();
				part->disk = this;
				part->lbaStart = entries[i].lbaStart;
				part->lbaLen = entries[i].lbaLen;
				part->letter = nextLetter++;
				partitions.push_back(part);
			}
		delete[] bootSector;
	}
}