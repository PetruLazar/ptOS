#include "disk.h"

#include "ide.h"
#include "ahci.h"

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
	void ControllerDetected(PCILocation pciLocation, DeviceHeader *header)
	{
		switch ((MassStorageController)header->subclass)
		{
		case MassStorageController::IDEController:
			IDE::ControllerDetected(pciLocation, header);
			break;
		case MassStorageController::SATAcontroller:
			AHCI::ControllerDetected(pciLocation, header);
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
					delete[] bootSector;
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