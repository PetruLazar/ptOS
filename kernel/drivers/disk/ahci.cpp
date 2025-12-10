#include "ahci.h"
#include "disk.h"
#include "../../utils/isriostream.h"
#include "../../core/sys.h"
// #include <iostream.h>
#include "../../core/paging.h"
#include "../../core/mem.h"
#include "../../core/filesystem/filesystem.h"
#include "../../core/scheduler.h"

using namespace Disk;
using namespace PCI;
using namespace std;

// tmp
#define SATA_SIG_ATA 0x00000101	  // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB 0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101	  // Port multiplier

// ahci specific syscalls
constexpr uint SYSCALL_DISK_AHCI_GETINFO = SYSCALL_DISK_DRIVER_INTERNAL | SYSCALL_DISK_AHCI_SPECIFIC | 0;

namespace AHCI
{
	inline result getInfo(StorageDevice *device, byte buffer[512])
	{
		word res;
		asm volatile(
			"int 0x30"
			: "=a"(res)
			: "a"(SYSCALL_DISK), "b"(SYSCALL_DISK_AHCI_GETINFO), "D"(device), "S"(buffer));
		return (result)res;
	}

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

	// last syscall data for irq handler
	bool waiting = false;
	byte port = 0;
	Thread *kernelThread, *blockedThread;

	struct StorageDevice : public Disk::StorageDevice
	{
	private:
	public:
		Controller *volatile controller;
		ull size;
		byte portNr;
		char model[41];

		int findCmdSlot(volatile Port &port)
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
		result prepCmd(CmdHeader *&cmdHeader, volatile Port &port, int &slot)
		{
			//  find cmd slot
			slot = findCmdSlot(port);
			if (slot == -1)
				return result::unknownError;

			cmdHeader = &port.cmdListBase[slot];
			return result::success;
		}
		result sendCmd(registers_t &regs, volatile Port &port, int slot)
		{
			// wait for port to not be busy
			int timeout = 0;
			while ((port.taskFileData & (Port::ATA_DEV_BUSY | Port::ATA_DEV_DRQ)) && timeout < 1000000)
			{
				ISR::std::cout << "AHCI disk is busy\n";
				timeout++;
			}
			if (timeout == 1000000)
				return result::deviceFault;

			port.commandIssue |= 1 << slot;

			// wait for completion
			Thread *callingThread = Scheduler::getCurrentThread();
			if (Scheduler::waitForThread(regs, kernelThread))
			{
				waiting = true;
				AHCI::port = portNr;
				blockedThread = callingThread;
				return result::success;
			}
			return result::unknownError;
		}
		virtual ull getSize() override { return size; }
		virtual string getModel() override { return model; }
		virtual string getLocation() override
		{
			return "Slot " + to_string(portNr) + " of AHCI Controller on " + controller->pciLocation.to_string();
		}
		result getIdentification(registers_t &regs, byte buffer[512])
		{
			volatile Port &port = controller->regs->ports[portNr];

			CmdHeader *cmdHeader;
			int slot;
			result res = prepCmd(cmdHeader, port, slot);
			if (res != result::success)
				return res;

			cmdHeader->setFISdwordCount(sizeof(FIS_reg_h2d) / sizeof(uint));
			cmdHeader->setWrite(false); // set according to dir
			cmdHeader->prdtLength = 1;

			CmdTable *cmdTable = cmdHeader->cmdTable;
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

			return sendCmd(regs, port, slot);
		}
		virtual result driver_access(registers_t &regs, accessDir dir, uint lba, uint numsec, byte *buffer) override
		{
			// read-only for now
			// if (dir == accessDir::write)
			// {
			// 	ISR::std::cout << "AHCI driver is read-only so far\n";
			// 	return result::unknownError;
			// }

			volatile Port &port = controller->regs->ports[portNr];

			CmdHeader *cmdHeader;
			int slot;
			result res = prepCmd(cmdHeader, port, slot);
			if (res != result::success)
				return res;

			// prepare header
			cmdHeader->setFISdwordCount(sizeof(FIS_reg_h2d) / sizeof(uint));
			cmdHeader->setWrite(false); // set according to dir
			cmdHeader->prdtLength = (word)(numsec - 1 >> 4) + 1;
			cmdHeader->prdByteCount = 0;

			CmdTable *cmdTable = cmdHeader->cmdTable;
			memset(cmdTable, sizeof(CmdTable) + (cmdHeader->prdtLength - 1) * sizeof(PRDTentry), 0);

			// fill PRDT: 8KB = 16 sectors per entry
			int i;
			for (i = 0; i < (int)cmdHeader->prdtLength - 1; i++)
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

			return sendCmd(regs, port, slot);
		}
	};

	void EventHandler(registers_t &regs);

	void Initialize()
	{
		controllers = new vector<Controller *>;

		// temporary: the interrupt number could be different than 11
		IRQ::registerIrqHandler(11, EventHandler);

		kernelThread = Scheduler::getCurrentThread();
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
		PCIDeviceHeader *header = &device.header;

		Controller *controller = new Controller;
		controllers->push_back(controller);
		controller->regs = (Registers *)(ull)header->bar5;
		controller->pciLocation = device.location;

		controller->pageSpace = (byte *)Memory::Allocate(0x8000, 0x1000);
		ull freeSpace = (ull)controller->pageSpace;

		PageMapLevel4 &current = PageMapLevel4::getCurrent();
		ull physicalAddress;
		if (!current.getPhysicalAddress((ull)header->bar5, physicalAddress))
		{
			void *pageSpace;
			dword pageAllocationMap;
			Memory::GetPageSpace(pageSpace, pageAllocationMap);
			if (!current.mapRegion(pageSpace, pageAllocationMap, (ull)header->bar5, (ull)header->bar5, 0x2000, true, false))
			{
				// handle error case...
				System::blueScreen();
			}
		}

		// preform bios handoff if supported

		// enable global AHCI interrupts
		controller->regs->interruptStatus = controller->regs->interruptStatus;
		controller->regs->globalHostControl |= 0x02;

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

					// turn on interrupts for port
					port.interruptEnable |= (1 << 0) |
											(1 << 1) |
											(1 << 2) |
											(1 << 3) |
											(1 << 4) |
											(1 << 5) |
											(1 << 6) |
											(1 << 7) |
											(1 << 22) |
											(1 << 23) |
											(1 << 24) |
											(1 << 26) |
											(1 << 27) |
											(1 << 28) |
											(1 << 29) |
											(1 << 30) |
											(1 << 31);
					port.interruptStatus = port.interruptStatus;

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
					if (getInfo(dev, (byte *)identify) == result::success)
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

		// cout << "Hello AHCI!\n";
		// cout << "\theader->interruptPin = " << header->interruptPin << '\n';
		// cout << "\theader->interruptLine = " << header->interruptLine << '\n';
		// cout << "\theader->command bit 10 = " << ((header->command >> 10) & 1) << '\n';
		// cout << "\tcontroller->regs->globalHostControl bit 1 = " << ((controller->regs->globalHostControl >> 1) & 1) << '\n';
		// cout << "\tcontroller->regs->portImplemented = " << ostream::base::bin << controller->regs->portImplemented << ostream::base::dec << '\n';
		// for (uint i = 0; i < 32; i++)
		// 	if ((controller->regs->portImplemented >> i) & 1)
		// 	{
		// 		cout << "\tport[" << i << "].interruptEnable = "
		// 			 << ostream::base::bin << controller->regs->ports[i].interruptEnable << ostream::base::dec << '\n';
		// 		// controller->regs->ports[i].interruptStatus = 0;
		// 	}
	}

	void EventHandler(registers_t &regs)
	{
		bool handled = false;

		// poll controllers
		for (auto *controller : *controllers)
		{
			// get controller interrupt status
			uint controllerInterruptStatus = controller->regs->interruptStatus;

			for (uint i = 0; i < 32; i++)
			{
				// check if port needs servicing
				if (controllerInterruptStatus & (1 << i))
				{
					// get port structure
					Port &port = controller->regs->ports[i];

					// get port interrupt status
					uint portInterruptStatus = port.interruptStatus;

					if (!waiting)
					{
						ISR::std::cout << "AHCI IRQ while not waiting for anything...\n";
						while (true)
							;
						return; // too many irqs...
					}

					waiting = false;

					if (portInterruptStatus & Port::TFES)
						blockedThread->getRegs().rax = (word)result::deviceFault;
					else
						blockedThread->getRegs().rax = (word)result::success;
					handled = true;

					// clear interrupt status
					port.interruptStatus = portInterruptStatus;
				}
			}

			// clear interrupt status
			controller->regs->interruptStatus = controllerInterruptStatus;

			if (handled)
				Scheduler::unblockThread(regs, kernelThread, blockedThread);
		}

		// get controller
		// volatile Controller *controller = controllers->at(0);
		// // get interrupt status registers
		// uint controllerInterruptStatus = controller->regs->interruptStatus,
		// 	 portInterruptStatus = controller->regs->ports[0].interruptStatus;
		// // clear interrupt flags
		// controller->regs->interruptStatus = controllerInterruptStatus;
		// controller->regs->ports[0].interruptStatus = portInterruptStatus;
		// if (!waiting)
		// {
		// 	ISR::std::cout << "AHCI IRQ while not waiting for anything...\n";
		// 	while (true)
		// 		;
		// 	return; // too many irqs...
		// }
		// // actual handling of the interrupt
		// waiting = false;
		// if (portInterruptStatus & Port::TFES)
		// 	blockedThread->getRegs().rax = (word)result::deviceFault;
		// else
		// 	blockedThread->getRegs().rax = (word)result::success;
		// Scheduler::unblockThread(regs, kernelThread, blockedThread);
	}

	void Syscall(registers_t &regs)
	{
		switch (regs.rbx)
		{
		case SYSCALL_DISK_AHCI_GETINFO:
			StorageDevice *device = (StorageDevice *)regs.rdi;
			byte *buffer = (byte *)regs.rsi;

			// buffer needs translation and validation before actual operation
			result res = device->getIdentification(regs, buffer);
			if (res != result::success)
				regs.rax = (word)res;
			return;
		}
	}
}
