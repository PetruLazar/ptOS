#include "ahci.h"
#include "disk.h"
#include <iostream.h>
#include "../../core/paging.h"
#include "../../core/mem.h"
#include "../../core/filesystem.h"

using namespace Disk;
using namespace PCI;
using namespace std;

// tmp
#define SATA_SIG_ATA 0x00000101	  // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101	  // Port multiplier

namespace AHCI
{
	enum class ATAcmd : byte
	{
		readPIO = 0x20,
		readPIOext = 0x24,
		readDMA = 0xc8,
		readDMAext = 0x25,
		writePIO = 0x30,
		writePIOext = 0x34,
		writeDMA = 0xca,
		writeDMAext = 0x35,
		cacheFlush = 0xe7,
		cacheFlushExt = 0xea,
		packet = 0xa0,
		identify_packet = 0xa1,
		identify = 0xec
	};
	enum class FIStype : byte
	{
		registerH2D = 0x27,
		registerD2H = 0x34,
		DMAact = 0x39,
		DMAsetup = 0x41,
		data = 0x46,
		BIST = 0x58,
		PIOsetup = 0x5f,
		devBits = 0xa1
	};

	struct FIS_reg_h2d
	{
		static constexpr byte cmd = 0x80;

		FIStype type;
		byte first, command, featureLow,

			lba0, lba1, lba2, device,

			lba3, lba4, lba5, featureHigh;

		word count;
		byte isoCmdCompletion, control,

			reserved[4];

		bool getCmd() { return first & cmd; }
		void setCmd(bool c) { c ? first |= cmd : first &= ~cmd; }
	};

	struct PRDTentry
	{
		static constexpr uint byteCountMask = 0x3fffff;
		static constexpr uint interruptOnCompletion = 0x80000000;

		ull dataAddress;
		uint reserved;
		uint dw3;

		uint getByteCount() { return dw3 & byteCountMask; }
		void setByteCount(uint val) { dw3 = dw3 & ~byteCountMask | val; }
		bool getInterruptOnCompletion() { return dw3 & interruptOnCompletion; }
		void setInterruptOnCompletion(bool i) { i ? dw3 |= interruptOnCompletion : dw3 &= ~interruptOnCompletion; }
	};
	struct CmdTable
	{
		byte cmdFIS[64],
			ATAPIcmd[16],
			reserved[48];
		PRDTentry prdt[1];
	};
	struct CmdHeader
	{
		static constexpr byte cmdFISlenMask = 0x1f,
							  ATAPI = 0x20,
							  write = 0x40,
							  prefetchable = 0x80,

							  reset = 0x1,
							  BIST = 0x2,
							  clearBusyUponROK = 0x4,
							  portMPport = 0xf0;

		byte first, second;
		word prdtLength;
		uint prdByteCount;
		CmdTable *cmdTable;
		uint reserved[4];

		byte getFISdwordCount() { return first & cmdFISlenMask; }
		void setFISdwordCount(byte value) { first = first & ~cmdFISlenMask | value; }
		bool getATAPI() { return first & ATAPI; }
		void setATAPI(bool a) { a ? first |= ATAPI : first &= ~ATAPI; }
		bool getWrite() { return first & write; }
		void setWrite(bool w) { w ? first |= write : first &= ~write; }
		bool getPrefetchable() { return first & prefetchable; }
		void setPrefethable(bool p) { p ? first |= prefetchable : first &= ~prefetchable; }

		// getters and setters for "second" bit fields
	};
	struct RecdFIS
	{
	};
	struct Port
	{
		static constexpr uint ATA_DEV_BUSY = 0x80,
							  ATA_DEV_DRQ = 0x08,
							  TFES = 0x40000000,
							  START = 1,
							  FIS_REC_ENABLED = 1 << 4;

		// uint cmdListBaseLow,
		// 	cmdListBaseHigh,
		// 	FISbaseLow,
		// 	FISbaseHigh;
		CmdHeader *cmdListBase;
		ull FISbase;
		uint interruptStatus,
			interruptEnable,
			cmdAndStatus,
			reserved1,
			taskFileData,
			signature,
			SATAstatus,
			SATAcontrol,
			SATAerror,
			SATAactive,
			commandIssue,
			SATAnotification,
			FISbasedSwitchControl,
			reserved2[11],
			vendorSpecific[4];
	};
	struct Registers
	{
		uint capability,
			globalHostControl,
			interruptStatus,
			portImplemented,
			version,
			commandCompletionCoalescingControl,
			commandCompletionCoalescingPorts,
			enclosureManagementLocation,
			enclosureManagementControl,
			capability2,
			handoffControlAndStatus;

		byte reserved[0xa0 - 0x2c],
			vendorSpecific[0x100 - 0xa0];

		Port ports[1];
	};

	struct Controller
	{
		Registers *regs;
		byte *pageSpace;
		vector<void *> allocations;
		uint cmdSlots;

		// location info
		PCILocation pciLocation;
	};
	vector<Controller *> *controllers;

	struct StorageDevice : public Disk::StorageDevice
	{
		Controller *controller;
		ull size;
		byte portNr;
		char model[41];

		int findCmdSlot(Port &port)
		{
			uint slots = port.SATAactive | port.commandIssue;
			for (uint i = 0; i < controller->cmdSlots; i++)
			{
				if ((slots & 1) == 0)
					return i;
				slots >>= 1;
			}
			return -1;
		}

		virtual ull getSize() override { return size; }
		virtual string getModel() override { return model; }
		virtual string getLocation() override
		{
			return "Slot " + to_string(portNr) + " of AHCI Controller on " + controller->pciLocation.to_string();
		}
		virtual bool getID(byte buffer[512])
		{
			Port &port = controller->regs->ports[portNr];
			port.interruptStatus = (uint)-1;
			// find cmd slot
			int slot = findCmdSlot(port);
			if (slot == -1)
				return false;

			// prepare header
			CmdHeader &cmdHeader = port.cmdListBase[slot];
			cmdHeader.setFISdwordCount(sizeof(FIS_reg_h2d) / sizeof(uint));
			cmdHeader.setWrite(false); // set according to dir
			cmdHeader.prdtLength = 1;

			CmdTable *cmdTable = cmdHeader.cmdTable;
			memset(cmdTable, sizeof(CmdTable), 0);

			// fill PRDT: 8KB = 16 sectors per entry
			// fill last entry PRDT
			PRDTentry &entry = cmdTable->prdt[0];
			entry.dataAddress = (ull)buffer;
			entry.setByteCount(511);
			entry.setInterruptOnCompletion(true);

			FIS_reg_h2d &fis = *(FIS_reg_h2d *)cmdTable->cmdFIS;
			fis.type = FIStype::registerH2D;
			fis.setCmd(true);
			fis.command = (byte)ATAcmd::identify;

			fis.lba0 = 0;
			fis.lba1 = 0;
			fis.lba2 = 0;
			fis.device = 1 << 6;

			fis.lba3 = 0;
			fis.lba4 = 0;
			fis.lba5 = 0;

			fis.count = 1;

			// wait for port to not be busy
			int timeout = 0;
			while ((port.taskFileData & (Port::ATA_DEV_BUSY | Port::ATA_DEV_DRQ)) && timeout < 1000000)
				timeout++;
			if (timeout == 1000000)
				return false;

			port.commandIssue = 1 << slot;

			// wait for completion
			while (true)
			{
				if ((port.commandIssue & (1 << slot)) == 0)
					break;
				if (port.interruptStatus & Port::TFES)
					return false;
			}
			if (port.interruptStatus & Port::TFES)
				return false;

			return true;
		}
		virtual result access(accessDir dir, uint lba, uint numsec, byte *buffer) override
		{
			// read-only for now
			if (dir == accessDir::write)
			{
				cout << "AHCI driver is read-only so far\n";
				return result::unknownError;
			}

			Port &port = controller->regs->ports[portNr];
			port.interruptStatus = (uint)-1;
			// find cmd slot
			int slot = findCmdSlot(port);
			if (slot == -1)
				return result::unknownError;

			// prepare header
			CmdHeader &cmdHeader = port.cmdListBase[slot];
			cmdHeader.setFISdwordCount(sizeof(FIS_reg_h2d) / sizeof(uint));
			cmdHeader.setWrite(false); // set according to dir
			cmdHeader.prdtLength = (word)(numsec - 1 >> 4) + 1;
			cmdHeader.prdByteCount = 0;

			CmdTable *cmdTable = cmdHeader.cmdTable;
			memset(cmdTable, sizeof(CmdTable) + (cmdHeader.prdtLength - 1) * sizeof(PRDTentry), 0);

			// fill PRDT: 8KB = 16 sectors per entry
			int i;
			for (i = 0; i < (int)cmdHeader.prdtLength - 1; i++)
			{
				PRDTentry &entry = cmdTable->prdt[i];
				entry.dataAddress = (ull)buffer;
				entry.setByteCount(8 * 1024 - 1);
				entry.setInterruptOnCompletion(true);
				buffer += 8 * 1024;
				// numsec -= 16;
			}
			// last entry
			PRDTentry &entry = cmdTable->prdt[i];
			entry.dataAddress = (ull)buffer;
			entry.setByteCount(((numsec - i * 16) << 9) - 1);
			entry.setInterruptOnCompletion(true);

			FIS_reg_h2d &fis = *(FIS_reg_h2d *)cmdTable->cmdFIS;
			fis.type = FIStype::registerH2D;
			fis.setCmd(true);
			fis.command = (byte)(dir == accessDir::read ? ATAcmd::readDMAext : ATAcmd::writeDMAext);

			fis.lba0 = lba;
			fis.lba1 = lba >> 8;
			fis.lba2 = lba >> 16;
			fis.device = 1 << 6;

			fis.lba3 = lba >> 24;
			fis.lba4 = 0;
			fis.lba5 = 0;

			fis.count = numsec;

			// wait for port to not be busy
			int timeout = 0;
			while ((port.taskFileData & (Port::ATA_DEV_BUSY | Port::ATA_DEV_DRQ)) && timeout < 1000000)
				timeout++;
			if (timeout == 1000000)
				return result::deviceFault;

			port.commandIssue |= 1 << slot;

			// wait for completion
			while (true)
			{
				if ((port.commandIssue & (1 << slot)) == 0)
					break;
				if (port.interruptStatus & Port::TFES)
					return result::deviceFault;
			}
			if (port.interruptStatus & Port::TFES)
				return result::deviceFault;

			return result::success;
		}
	};

	void Initialize()
	{
		controllers = new vector<Controller *>;
	}
	void CleanUp()
	{
		for (auto &controller : *controllers)
		{
			delete[] controller->pageSpace;
			for (auto &ptr : controller->allocations)
				delete[] (byte *)ptr;
			delete controller;
		}
		delete controllers;
	}

	void ControllerDetected(PCIDevice &device)
	{
		Controller *controller = new Controller;
		controllers->push_back(controller);
		controller->regs = (Registers *)(ull)header->bar5;
		controller->pciLocation = device.location;

		controller->pageSpace = (byte *)Memory::Allocate(0x8000, 0x1000);
		ull freeSpace = (ull)controller->pageSpace;

		PageMapLevel4 &current = PageMapLevel4::getCurrent();
		ull physicalAddress;
		if (!current.getPhysicalAddress((ull)header->bar5, physicalAddress))
			current.mapRegion(freeSpace, (ull)header->bar5, (ull)header->bar5, 0x2000, true, false);

		// preform bios handoff is supported

		uint implementedPorts = controller->regs->portImplemented;
		controller->cmdSlots = ((controller->regs->capability >> 8) & 0x1f) + 1;
		if (controller->cmdSlots > 2)
			controller->cmdSlots = 2;
		for (uint i = 0; i < 32; i++)
		{
			if (implementedPorts & 1)
			{
				// port is implemented
				Port &port = controller->regs->ports[i];
				uint SATAStatus = port.SATAstatus;
				byte det = SATAStatus & 0xf,
					 ipm = SATAStatus >> 8 & 0xf;

				if (det != 3 || ipm != 1)
					continue;

				// device detected
				uint sig = port.signature;

				// display device info
				switch (sig)
				{
				case SATA_SIG_PM:
					break;
				case SATA_SIG_ATAPI:
					break;
				case SATA_SIG_SEMB:
					break;
				default:
				{
					// build device
					StorageDevice *dev = new StorageDevice;
					dev->controller = controller;
					dev->portNr = i;
					devices->push_back(dev);

					// allocate memory for the device if it is not
					if (!port.cmdListBase)
					{
						// cout << "DEBUG: Allocated a command list.\n";
						void *ptr = Memory::Allocate(0x400, 0x400);
						controller->allocations.push_back(ptr);
						port.cmdListBase = (CmdHeader *)ptr;
					}
					if (!port.FISbase)
					{
						// cout << "DEBUG: Allocated a FIS struct.\n";
						void *ptr = Memory::Allocate(0x100, 0x100);
						controller->allocations.push_back(ptr);
						port.cmdListBase = (CmdHeader *)ptr;
					}
					for (uint i = 0; i < controller->cmdSlots; i++)
						if (!port.cmdListBase[i].cmdTable)
						{
							// cout << "DEBUG: Allocated CMD table for cmd slot " << i << ".\n";
							void *ptr = Memory::Allocate(0x10080, 0x80);
							controller->allocations.push_back(ptr);
							port.cmdListBase[i].cmdTable = (CmdTable *)ptr;
						}

					// check device status
					if (!(port.cmdAndStatus & Port::FIS_REC_ENABLED))
						port.cmdAndStatus |= Port::FIS_REC_ENABLED;
					if (!(port.cmdAndStatus & Port::START))
						port.cmdAndStatus |= Port::START;

					// identify cmd
					word *identify = new word[256];
					if (dev->getID((byte *)identify))
					{
						if (identify[83] & (1 << 10))
							// lba48
							dev->size = *(ull *)&identify[100];
						else
							// lba28
							dev->size = *(uint *)&identify[60];

						for (int k = 0; k < 40; k += 2)
						{
							dev->model[k] = ((char *)identify)[(word)IDfield::model + k + 1];
							dev->model[k + 1] = ((char *)identify)[(word)IDfield::model + k];
						}
						dev->model[40] = 0;
					}
					else
					{
						dev->size = 0;
						const char unk[] = "Unknown device";
						memcpy(dev->model, unk, sizeof(unk));
						delete[] identify;
						continue;
					}
					delete[] identify;

					dev->detectPartitions();
					Filesystem::detectPartitions(dev);
				}
				}
			}

			implementedPorts >>= 1;
		}
	}
}