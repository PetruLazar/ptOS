#include "ide.h"
#include "disk.h"
#include "../../cpu/ports.h"
#include <iostream.h>
#include "../../cpu/idt.h"
#include "../../core/sys.h"
#include "../../core/filesystem/filesystem.h"

using namespace PCI;
using namespace Disk;
using namespace std;

namespace IDE
{
	static constexpr word primary_io = 0x1f0,
						  primary_ctrl = 0x3f6,
						  secondary_io = 0x170,
						  secondary_ctrl = 0x376;

	enum class ATAstatus : byte
	{
		busy = 0x80,
		driveReady = 0x40,
		driveWriteFault = 0x20,
		driveSeekComplete = 0x10,
		dataRequestReady = 0x8,
		correctedData = 0x4,
		index = 0x2,
		error = 0x1
	};
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
	enum class ATAreg : byte
	{
		// bar0/2 offset
		data = 0x0,
		error = 0x1,
		features = 0x1,
		seccount0 = 0x2,
		lba0 = 0x3,
		lba1 = 0x4,
		lba2 = 0x5,
		hddevsel = 0x6,
		command = 0x7,
		status = 0x7,

		// ???
		seccount1 = 0x8,
		lba3 = 0x9,
		lba4 = 0xa,
		lba5 = 0xb,

		// bar1/3 offset-0xc
		control = 0xc,
		altstatus = 0xc,
		devaddress = 0xd
	};

	dword getIdField(void *idSpace, IDfield field)
	{
		switch (field)
		{
		case IDfield::deviceType:
		case IDfield::capabilities:
			return *(word *)((byte *)idSpace + (word)field);
		case IDfield::commandSets:
		case IDfield::maxLba:
		case IDfield::maxLbaExt:
			return *(dword *)((byte *)idSpace + (word)field);
		}
		return 0xffffffff;
	}

	class IDEController
	{
	public:
		struct IDEchannel
		{
			word ioBase;
			word ctrl;
		} channels[2];

		// location info
		PCILocation pciLocation;

		byte readReg(byte channel, ATAreg reg)
		{
			if (reg <= ATAreg::status)
				return inb(channels[channel].ioBase + (byte)reg);
			return inb(channels[channel].ctrl + (byte)reg - 0xc);
		}
		word readRegW(byte channel, ATAreg reg)
		{
			if (reg <= ATAreg::status)
				return inw(channels[channel].ioBase + (byte)reg);
			return inw(channels[channel].ctrl + (byte)reg - 0xc);
		}
		dword readRegDW(byte channel, ATAreg reg)
		{
			if (reg <= ATAreg::status)
				return indw(channels[channel].ioBase + (byte)reg);
			return indw(channels[channel].ctrl + (byte)reg - 0xc);
		}
		void writeReg(byte channel, ATAreg reg, byte val)
		{
			if (reg <= ATAreg::status)
				outb(channels[channel].ioBase + (byte)reg, val);
			else if (reg <= ATAreg::lba5)
				outb(channels[channel].ioBase - (byte)ATAreg::seccount1 + (byte)ATAreg::seccount0, val);
			else
				outb(channels[channel].ctrl + (byte)reg - 0xc, val);
		}
		void writeRegW(byte channel, ATAreg reg, word val)
		{
			if (reg <= ATAreg::status)
				outw(channels[channel].ioBase + (byte)reg, val);
			else
				outw(channels[channel].ctrl + (byte)reg - 0xc, val);
		}

		void sleep1ms(byte channelToUse)
		{
			for (int i = 0; i < 10; i++)
				readReg(channelToUse, ATAreg::altstatus);
		}
		result polling(byte channel, bool advanced_check)
		{
			for (int i = 0; i < 4; i++)
				readReg(channel, ATAreg::altstatus);

			byte status;
			do
			{
				status = readReg(channel, ATAreg::status);
			} while (status & (byte)ATAstatus::busy);

			if (advanced_check)
			{
				if (status & (byte)ATAstatus::error)
				{
					byte err = readReg(channel, ATAreg::error);
					return (result)(err << 8 | (word)result::errorRegisterSet);
				}
				if (status & (byte)ATAstatus::driveWriteFault)
					return result::deviceFault;
				if (status & (byte)ATAstatus::dataRequestReady)
					return result::success;
				return result::unknownError;
			}
			return result::success;
		}

		int DetectDisks();
	};

	class ATADevice : public StorageDevice
	{
	public:
		IDEController *controller;

		byte channel, drive;
		IDEtype type;
		word signature, capabilities;
		dword commandSets, size;
		char model[41];

		virtual ull getSize() override { return size; }
		virtual string getModel() override { return model; }
		virtual string getLocation() override
		{
			return "Channel " + to_string(channel) + ", drive " + to_string(drive) + " of IDE Controller on " + controller->pciLocation.to_string();
		}

		virtual result access(accessDir dir, uint lba, uint numsects, byte *buffer_) override
		{
			accessMode mode;
			bool dma;
			ATAcmd cmd;
			byte data[6];
			word ioBase = controller->channels[channel].ioBase, cyl;
			byte head, sect;
			constexpr word words = 256; // 512 byte sector size

			if (lba >= 0x10000000)
			{
				// lba48, assuming the device supports lba, since the lba is impossible otherwise
				mode = accessMode::lba48;
				data[0] = byte(lba & 0xff);
				data[1] = byte((lba >> 8) & 0xff);
				data[2] = byte((lba >> 16) & 0xff);
				data[3] = byte((lba >> 24) & 0xff);
				data[4] = 0;
				data[5] = 0;
				head = 0;
			}
			else if (capabilities & 0x200)
			{
				// lba is supported, using lba28
				mode = accessMode::lba28;
				data[0] = byte(lba & 0xff);
				data[1] = byte((lba >> 8) & 0xff);
				data[2] = byte((lba >> 16) & 0xff);
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				head = byte((lba >> 24) & 0xf);
			}
			else
			{
				// using chs
				mode = accessMode::chs;
				sect = (lba % 63) + 1;
				cyl = (lba + 1 - sect) / (16 * 63);
				data[0] = sect;
				data[1] = byte(cyl & 0xff);
				data[2] = byte((cyl >> 8) & 0xff);
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				head = (lba + 1 - sect) % (16 * 63) / 63;
			}

			dma = false;

			while (controller->readReg(channel, ATAreg::status) & (byte)ATAstatus::busy)
				;

			controller->writeReg(channel, ATAreg::hddevsel, (mode == accessMode::chs ? 0xa0 : 0xe0) | (drive << 4) | head);
			if (mode == accessMode::lba48)
			{
				// to be tested, do not use
				cout << "Warning! LBA48 used!\n";
				controller->writeReg(channel, ATAreg::seccount1, 0);
				controller->writeReg(channel, ATAreg::lba3, data[3]);
				controller->writeReg(channel, ATAreg::lba4, data[4]);
				controller->writeReg(channel, ATAreg::lba5, data[5]);
			}
			controller->writeReg(channel, ATAreg::seccount0, numsects);
			controller->writeReg(channel, ATAreg::lba0, data[0]);
			controller->writeReg(channel, ATAreg::lba1, data[1]);
			controller->writeReg(channel, ATAreg::lba2, data[2]);

			if (dir == accessDir::read)
			{
				// read
				if (dma)
				{
					// dma
					if (mode == accessMode::lba48)
						cmd = ATAcmd::readDMAext;
					else
						cmd = ATAcmd::readDMA;
				}
				else
				{
					// pio
					if (mode == accessMode::lba48)
						cmd = ATAcmd::readPIOext;
					else
						cmd = ATAcmd::readPIO;
				}
			}
			else
			{
				// write
				if (dma)
				{
					// dma
					if (mode == accessMode::lba48)
						cmd = ATAcmd::writeDMAext;
					else
						cmd = ATAcmd::writeDMA;
				}
				else
				{
					// pio
					if (mode == accessMode::lba48)
						cmd = ATAcmd::writePIOext;
					else
						cmd = ATAcmd::writePIO;
				}
			}
			controller->writeReg(channel, ATAreg::command, (byte)cmd);

			word *buffer = (word *)buffer_;
			if (dma)
			{
				if (dir == accessDir::read)
					; // dma read
				else
					; // dma write
			}
			else
			{
				if (dir == accessDir::read)
				{
					result res;
					// pio read
					// int testvar = 0;
					for (int s = 0; s < (byte)numsects; s++)
					{
						res = controller->polling(channel, true);
						if (res != result::success)
						{
							// cout << "Throwing error " << err << '\n';
							return res;
						}
						for (int i = 0; i < words; i++)
							buffer[i] = controller->readRegW(channel, ATAreg::data);
						buffer += words;
						// testvar++;
					}
					// cout << "Actually read " << testvar << " sectors\n";
				}
				else
				{
					// pio write
					for (int s = 0; s < (byte)numsects; s++)
					{
						controller->polling(channel, false);
						for (int i = 0; i < words; i++)
							controller->writeRegW(channel, ATAreg::data, buffer[i]);
						buffer += words;
					}
					controller->writeReg(channel, ATAreg::command, byte(mode == accessMode::lba48 ? ATAcmd::cacheFlushExt : ATAcmd::cacheFlush));
					controller->polling(channel, false);
				}
			}
			return result::success;
		}
	};

	int IDEController::DetectDisks()
	{
		byte deviceCount = 0;

		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
			{
				byte err;
				IDEtype type = IDEtype::unknown;
				// Device &dev = devices[deviceCount];

				// int j = -rj + 1;
				// int j = rj;
				writeReg(i, ATAreg::hddevsel, 0xa0 | (j << 4));
				// sleep(1)
				sleep1ms(i);

				writeReg(i, ATAreg::command, (byte)ATAcmd::identify);
				// sleep(1)
				sleep1ms(i);

				byte status = readReg(i, ATAreg::status);
				if (status == 0)
					continue;

				while (true)
				{
					status = readReg(i, ATAreg::status);
					if (status & (byte)ATAstatus::error)
					{
						err = 1;
						break;
					}
					if (!(status & (byte)ATAstatus::busy) && (status & (byte)ATAstatus::dataRequestReady))
						break;
					// status = readReg(i, ATAreg::status);
				}

				byte cl = readReg(i, ATAreg::lba1),
					 ch = readReg(i, ATAreg::lba2);
				if (err != 0)
				{
					if (cl == 0x14 && ch == 0xeb)
						type = IDEtype::PATAPI;
					else if (cl == 0x69 && ch == 0x96)
						type = IDEtype::SATAPI;
					else
						continue;

					writeReg(i, ATAreg::command, (byte)ATAcmd::identify_packet);
					// sleep(1);
					sleep1ms(i);
				}
				else
				{
					if (cl == 0 && ch == 0)
						type = IDEtype::PATA;
					else if (cl == 0x3c && ch == 0xc3)
						type = IDEtype::SATA;
					else
						continue;
				}

				word *identificationSpace = new word[256];
				for (int offset = 0; offset < 256; offset++)
					identificationSpace[offset] = readRegW(i, ATAreg::data);

				ATADevice *dev = new ATADevice();
				devices->push_back(dev);
				deviceCount++;
				dev->controller = this;
				dev->type = type;
				dev->channel = i;
				dev->drive = j;
				dev->signature = getIdField(identificationSpace, IDfield::deviceType);
				dev->capabilities = getIdField(identificationSpace, IDfield::capabilities);
				dev->commandSets = getIdField(identificationSpace, IDfield::commandSets);
				if (dev->commandSets & (1 << 26))
					dev->size = getIdField(identificationSpace, IDfield::maxLbaExt);
				else
					dev->size = getIdField(identificationSpace, IDfield::maxLba);

				/*dev.size = getIdField(identificationSpace,
									  (dev.commandSets & (1 << 26)) ? IDfield::maxLbaExt : IDfield::maxLba);*/

				for (int k = 0; k < 40; k += 2)
				{
					dev->model[k] = ((char *)identificationSpace)[(word)IDfield::model + k + 1];
					dev->model[k + 1] = ((char *)identificationSpace)[(word)IDfield::model + k];
				}
				dev->model[40] = 0;
				delete[] identificationSpace;

				if (dev->type == IDEtype::PATA || dev->type == IDEtype::SATA)
				{
					dev->detectPartitions();
					Filesystem::detectPartitions(dev);
				}
			}
		return deviceCount;
	}

	vector<IDEController *> *controllers;

	int CHStoLba(byte head, word cylAndSec)
	{
		word cyl = (cylAndSec >> 8) | (cylAndSec & 0xc0 << 2);
		byte sec = cylAndSec & 0x3f;
		return (cyl * 16 + head) * 63 + sec - 1;
	}
	/*int LbaToChs(int lba)
	{

	}*/

	const char *deviceTypes[] = {
		"SATA",
		"PATA",
		"SATAPI",
		"PATAPI",
		"unknown",
	};

	// bool irqRecd = false;
	// void irqReceive(registers_t &)
	// {
	// 	irqRecd = true;
	// }
	// void waitForIrq()
	// {
	// 	while (!irqRecd)
	// 		;
	// 	irqRecd = false;
	// }

	// void readIdSpace(byte channel, byte *buffer)
	// {
	// }

	/*dword getATAPIDeviceSize(byte drive)
	{
		Device &dev = devices[drive];
		byte channel = dev.channel,
			 slaveBit = dev.drive;
		// cout << "Started:\n";
		// irqRecd = false;
		writeReg(channel, ATAreg::features, 0);
		writeReg(channel, ATAreg::lba1, 0x08);
		writeReg(channel, ATAreg::lba2, 0x00);

		writeReg(channel, ATAreg::command, (byte)ATAcmd::packet);

		byte tmp = polling(channel, true);
		if (tmp)
		{
			cout << "Error 1:\n";
			displayError(drive, tmp);
			return 0;
		}

		byte packet[12] = {0x25, 0b00000000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00};
		for (int i = 0; i < 6; i++)
			writeRegW(channel, ATAreg::data, ((word *)packet)[i]);

		tmp = polling(channel, true);
		if (tmp)
		{
			cout << "Error 2:\n";
			displayError(drive, tmp);
			return 0;
		}
		while (true)
			;
		word result[4];
		for (int i = 0; i < 4; i++)
			result[i] = readRegW(channel, ATAreg::data);

		displayMemoryRow((byte *)result);

		qword retLba = ((dword *)result)[0],
			  blockLen = ((dword *)result)[1];

		return (retLba + 1) * blockLen;
	}*/

	void Initialize()
	{
		controllers = new vector<IDEController *>;

		// IDT::registerIrqHandler(14, irqReceive);
		// IDT::registerIrqHandler(15, irqReceive);

		// enable interrupts by clearing bit 1
		//  writeReg(0, ATAreg::control, 0); // ?
		//  writeReg(1, ATAreg::control, 0); // ?
	}
	void CleanUp()
	{
		for (auto &controller : *controllers)
			delete controller;
		delete controllers;
	}
	void ControllerDetected(PCIDevice &device)
	{
		// init controller structure
		IDEController *controller = new IDEController;
		PCIDeviceHeader *header = &device.header;
		controller->channels[0].ioBase = device.header.bar0 ? header->bar0 : primary_io;
		controller->channels[0].ctrl = header->bar1 ? header->bar1 : primary_ctrl;
		controller->channels[1].ioBase = header->bar2 ? header->bar2 : secondary_io;
		controller->channels[1].ctrl = header->bar3 ? header->bar3 : secondary_ctrl;
		controller->pciLocation = device.location;

		// stop the controller from triggering interrupts
		controller->writeReg(0, ATAreg::control, 2);
		controller->writeReg(1, ATAreg::control, 2);

		if (controller->DetectDisks())
			controllers->push_back(controller);
		else
		{
			delete controller;
		}
	}

	enum class ATAPIcmd
	{
		read = 0xa8,
		write = 0xaa,
	};
}