#include "disk.h"
#include "../cpu/ports.h"
#include "../utils/iostream.h"
#include "../cpu/idt.h"
#include "../core/sys.h"
#include "../core/filesystem.h"

using namespace std;

namespace Disk
{
	int CHStoLba(byte head, word cylAndSec)
	{
		word cyl = (cylAndSec >> 8) | (cylAndSec & 0xc0 << 2);
		byte sec = cylAndSec & 0x3f;
		return (cyl * 16 + head) * 63 + sec - 1;
	}
	/*int LbaToChs(int lba)
	{

	}*/

	static constexpr word primary_io = 0x1f0,
						  primary_ctrl = 0x3f6,
						  secondary_io = 0x170,
						  secondary_ctrl = 0x376;

	struct IDEchannel
	{
		word ioBase;
		word ctrl;
	} channels[2];

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
	enum class ATAerror : byte
	{
		badBlock = 0x80,
		uncorrectableData = 0x40,
		mediaChanged = 0x20,
		idMarkNotFound = 0x10,
		mediaChangeRequest = 0x8,
		commandAborted = 0x4,
		track0NotFound = 0x2,
		noAddressMark = 0x1
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

	const char *deviceTypes[] = {
		"SATA",
		"PATA",
		"SATAPI",
		"PATAPI",
		"unknown",
	};

	bool irqRecd = false;
	void irqReceive()
	{
		irqRecd = true;
	}
	void waitForIrq()
	{
		while (!irqRecd)
			;
		irqRecd = false;
	}

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
	byte polling(byte channel, bool advanced_check)
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
				return 2;
			if (status & (byte)ATAstatus::driveWriteFault)
				return 1;
			if (status & (byte)ATAstatus::dataRequestReady)
				return 0;
			return 3;
		}
		return 0;
	}
	void displayError(byte device, byte err)
	{
		switch (err)
		{
		case 0:
			cout << "No error\n";
			break;
		case 1:
			cout << "Device fault\n";
			break;
		case 2:
		{
			byte reg = readReg(devices[device].channel, ATAreg::error);
			cout << "Error register:\n";
			if (reg & (byte)ATAerror::badBlock)
				cout << "   - bad block\n";
			if (reg & (byte)ATAerror::commandAborted)
				cout << "   - command aborted\n";
			if (reg & (byte)ATAerror::idMarkNotFound)
				cout << "   - id mark not found\n";
			if (reg & (byte)ATAerror::mediaChanged)
				cout << "   - media changed\n";
			if (reg & (byte)ATAerror::mediaChangeRequest)
				cout << "   - media change request\n";
			if (reg & (byte)ATAerror::noAddressMark)
				cout << "   - no address mark\n";
			if (reg & (byte)ATAerror::track0NotFound)
				cout << "   - track 0 not found\n";
			if (reg & (byte)ATAerror::uncorrectableData)
				cout << "   - uncorrectable data\n";
		}
		break;
		case 3:
			cout << "Unknown error\n";
		}
	}
	void readIdSpace(byte channel, byte *buffer)
	{
	}

	enum class IDfield : word
	{
		deviceType = 0,
		cylinders = 2,
		heads = 6,
		sectors = 12,
		serial = 20,
		model = 54,
		capabilities = 98,
		fieldValid = 106,
		maxLba = 120,
		commandSets = 164,
		maxLbaExt = 200
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

	Device devices[4];

	void sleep1ms(byte channelToUse)
	{
		for (int i = 0; i < 10; i++)
			readReg(channelToUse, ATAreg::altstatus);
	}

	dword getATAPIDeviceSize(byte drive)
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
	}

	bool initialized = false;
	void Initialize(dword bar0, dword bar1, dword bar2, dword bar3, dword bar4, dword bar5)
	{
		// temp stub code
		if (initialized)
			return;
		else
			initialized = true;

		IDT::registerIrqHandler(14, irqReceive);
		IDT::registerIrqHandler(15, irqReceive);

		channels[0].ioBase = bar0 ? bar0 : primary_io;
		channels[0].ctrl = bar1 ? bar1 : primary_ctrl;
		channels[1].ioBase = bar2 ? bar2 : secondary_io;
		channels[1].ctrl = bar3 ? bar3 : secondary_ctrl;

		writeReg(0, ATAreg::control, 2);
		writeReg(1, ATAreg::control, 2);

		byte deviceCount = 0;

		for (int i = 0; i < 2; i++)
			for (int rj = 0; rj < 2; rj++)
			{
				byte err;
				IDEtype type = IDEtype::unknown;
				Device &dev = devices[deviceCount];

				dev.reserved = 0;
				int j = -rj + 1;
				// int j = rj;
				// writeReg(i, ATAreg::hddevsel, 0xa0 | (j << 4));
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

				dev.reserved = 1;
				dev.type = type;
				dev.channel = i;
				dev.drive = j;
				dev.signature = getIdField(identificationSpace, IDfield::deviceType);
				dev.capabilities = getIdField(identificationSpace, IDfield::capabilities);
				dev.commandSets = getIdField(identificationSpace, IDfield::commandSets);
				if (dev.commandSets & (1 << 26))
					dev.size = getIdField(identificationSpace, IDfield::maxLbaExt);
				else
					dev.size = getIdField(identificationSpace, IDfield::maxLba);

				/*dev.size = getIdField(identificationSpace,
									  (dev.commandSets & (1 << 26)) ? IDfield::maxLbaExt : IDfield::maxLba);*/

				for (int k = 0; k < 40; k += 2)
				{
					dev.model[k] = ((char *)identificationSpace)[(word)IDfield::model + k + 1];
					dev.model[k + 1] = ((char *)identificationSpace)[(word)IDfield::model + k];
				}
				dev.model[40] = 0;

				/*cout
					<< "Detected:\n   channel: " << ostream::base::hex << dev.channel
					<< "\n   drive: " << dev.drive
					<< "\n   type: " << deviceTypes[(byte)type]
					<< "\n   signature: " << dev.signature
					<< "\n   capabilities: " << dev.capabilities
					<< "\n   command sets: " << dev.commandSets
					<< "\n   max lba: " << getIdField(identificationSpace, IDfield::maxLba)
					<< "\n   max lba ext: " << getIdField(identificationSpace, IDfield::maxLbaExt)
					<< "\n   size: " << dev.size
					<< "\n   model: " << dev.model << ostream::base::dec << '\n';
				System::pause();*/

				// read sector 0 and tell the filesystem about the volumes present
				byte *bootsector = new byte[512];
				if (byte err = accessATAdrive(accessDir::read, deviceCount, 0, 1, bootsector) == 0)
				{
					Filesystem::detectPartitions(dev, bootsector);
				}
				else
				{
					// error: err
				}

				delete[] bootsector;

				deviceCount++;
			}

		// writeReg(0, ATAreg::control, 0); // ?
		// writeReg(1, ATAreg::control, 0); // ?

		cout << "Disk initialization complete.\n";
	}

	enum class ATAPIcmd
	{
		read = 0xa8,
		write = 0xaa,
	};

	/*byte readATAPI(byte drive, uint lba, byte numsects, void *buffer_)
	{
		// uses pio, slower, through ports
		// todo: implement dma: faster, writes directly into memory buffer and then triggers an interrupt

		Device &dev = devices[drive];

		if (!dev.reserved)
			return 0xff;

		byte channel = dev.channel,
			 slavebit = dev.drive;
		word base = channels[channel].ioBase,
			 *buffer = (word *)buffer_;
		// uint words = 1024; // why not 256?
		byte err;

		// enable irqs

		byte packet[12] = {
			(byte)ATAPIcmd::read,
			0,
			byte((lba >> 24) & 0xff),
			byte((lba >> 16) & 0xff),
			byte((lba >> 8) & 0xff),
			byte(lba & 0xff),
			0,
			0,
			0,
			numsects,
			0,
			0,
		};

		writeReg(channel, ATAreg::hddevsel, slavebit << 4);
		sleep1ms(channel);

		writeReg(channel, ATAreg::features, 0); // pio mode

		static constexpr uint words = 1024; // 2048 byte sectors
		writeReg(channel, ATAreg::lba1, (words * 2) & 0xff);
		writeReg(channel, ATAreg::lba2, (words * 2) >> 8);

		writeReg(channel, ATAreg::command, (byte)ATAcmd::packet);
		if (err = polling(channel, true))
			return err;

		for (int i = 0; i < 6; i++)
			outw(base, ((word *)packet)[i]);

		for (int i = 0; i < numsects; i++)
		{
			waitForIrq();
			if (err = polling(channel, true))
				return err;
			for (uint j = 0; j < words; j++)
				*(buffer++) = inw(base);
		}

		waitForIrq();
		while (readReg(channel, ATAreg::status) & ((byte)ATAstatus::busy | (byte)ATAstatus::dataRequestReady))
			;

		return 0;
	}*/
	byte accessATAdrive(accessDir dir, byte drive, uint lba, byte numsects, void *buffer_)
	{
		Device &dev = devices[drive];
		accessMode mode;
		bool dma;
		ATAcmd cmd;
		byte data[6],
			channel = dev.channel, slave = dev.drive;
		word ioBase = channels[channel].ioBase, cyl;
		byte head, sect, err;
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
		else if (dev.capabilities & 0x200)
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

		while (readReg(channel, ATAreg::status) & (byte)ATAstatus::busy)
			;

		writeReg(channel, ATAreg::hddevsel, (mode == accessMode::chs ? 0xa0 : 0xe0) | (slave << 4) | head);
		if (mode == accessMode::lba48)
		{
			// to be tested, do not use
			writeReg(channel, ATAreg::seccount1, 0);
			writeReg(channel, ATAreg::lba3, data[3]);
			writeReg(channel, ATAreg::lba4, data[4]);
			writeReg(channel, ATAreg::lba5, data[5]);
		}
		writeReg(channel, ATAreg::seccount0, numsects);
		writeReg(channel, ATAreg::lba0, data[0]);
		writeReg(channel, ATAreg::lba1, data[1]);
		writeReg(channel, ATAreg::lba2, data[2]);

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
		writeReg(channel, ATAreg::command, (byte)cmd);

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
				// pio read
				for (int s = 0; s < numsects; s++)
				{
					if (err = polling(channel, true))
						return err;
					for (int i = 0; i < words; i++)
						buffer[i] = readRegW(channel, ATAreg::data);
					buffer += words;
				}
			else
			{
				// pio write
				for (int s = 0; s < numsects; s++)
				{
					polling(channel, false);
					for (int i = 0; i < words; i++)
						writeRegW(channel, ATAreg::data, buffer[i]);
					buffer += words;
				}
				writeReg(channel, ATAreg::command, byte(mode == accessMode::lba48 ? ATAcmd::cacheFlushExt : ATAcmd::cacheFlush));
				polling(channel, false);
			}
		}
		return 0;
	}
}