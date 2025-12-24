#include "pci.h"
#include "disk/disk.h"
#include "audio.h"
#include "net.h"
#include "../core/sys.h"
#include "../debug/verbose.h"

using namespace std;

namespace PCI
{
	void classSubclassProgIF_toString(byte classCode, byte subclass, byte progIF, const char *strings[3])
	{
		switch (classCode)
		{
		case 0x0:
			strings[0] = "Unclassified";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Non-VGA-compatible unclassified device";
				break;
			case 0x1:
				strings[1] = "VGA-compatible unclassified device";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
		case 0x1:
			strings[0] = "Mass storage controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "SCSI bus controller";
				strings[2] = "Unused";
				break;
			case 0x1:
				strings[1] = "IDE controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "ISA compatibility mode only";
					break;
				case 0x5:
					strings[2] = "PCI native mode-only";
					break;
				case 0xa:
					strings[2] = "ISA combatibility mode, both channels can be switched to PCI native mode";
					break;
				case 0xf:
					strings[2] = "PCI native mode, both channels can be switched to ISA compatibility mode";
					break;
				case 0x80:
					strings[2] = "ISA compatibility mode only, supports bus mastering";
					break;
				case 0x85:
					strings[2] = "PCI native mode-only, supports bus mastering";
					break;
				case 0x8a:
					strings[2] = "ISA combatibility mode, both channels can be switched to PCI native mode, supports bus mastering";
					break;
				case 0x8f:
					strings[2] = "PCI native mode, both channels can be switched to ISA compatibility mode, supports bus mastering";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x2:
				strings[1] = "Floppy disk controller";
				strings[2] = "Unused";
				break;
			case 0x3:
				strings[1] = "IPI bus controller";
				strings[2] = "Unused";
				break;
			case 0x4:
				strings[1] = "RAID controller";
				strings[2] = "Unused";
				break;
			case 0x5:
				strings[1] = "ATA controller";
				switch (progIF)
				{
				case 0x20:
					strings[2] = "Single DMA";
					break;
				case 0x30:
					strings[2] = "Chained DMA";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x6:
				strings[1] = "Serial ATA controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Vendor specific interface";
					break;
				case 0x1:
					strings[2] = "AHCI 1.0";
					break;
				case 0x2:
					strings[2] = "Serial storage bus";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x7:
				strings[1] = "Serial Attached SCSI controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "SAS";
					break;
				case 0x1:
					strings[2] = "Serial storage bus";
					break;
				default:
					strings[2] = "unknown";
				}
				break;
			case 0x8:
				strings[1] = "Non-volatile memory controller";
				switch (progIF)
				{
				case 0x1:
					strings[2] = "NVMHCI";
					break;
				case 0x2:
					strings[2] = "NVM express";
					break;
				default:
					strings[2] = "unknown";
				}
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unknown";
				break;
			}
			break;
		case 0x2:
			strings[0] = "Network controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Ethernet controller";
				break;
			case 0x1:
				strings[1] = "Token ring controller";
				break;
			case 0x2:
				strings[1] = "RDDI controller";
				break;
			case 0x3:
				strings[1] = "ATM controller";
				break;
			case 0x4:
				strings[1] = "ISDN controller";
				break;
			case 0x5:
				strings[1] = "WorldFip controller";
				break;
			case 0x6:
				strings[1] = "PICMG 2.14 multi computing controller";
				break;
			case 0x7:
				strings[1] = "Infiniband controller";
				break;
			case 0x8:
				strings[1] = "Fabric controller";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x3:
			strings[0] = "Display controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "VGA-compatible controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "VGA controller";
					break;
				case 0x1:
					strings[2] = "8514-compatible controller";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x1:
				strings[1] = "XGA controller";
				strings[2] = "Unused";
				break;
			case 0x2:
				strings[1] = "3D controller (not VGA-compatible)";
				strings[2] = "Unused";
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0x4:
			strings[0] = "Multimedia controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Multimedia video controller";
				break;
			case 0x1:
				strings[1] = "Multimedia audio controller";
				break;
			case 0x2:
				strings[1] = "Computer telephony device";
				break;
			case 0x3:
				strings[1] = "Audio device";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x5:
			strings[0] = "Memory controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "RAM controller";
				break;
			case 0x1:
				strings[1] = "Flash controller";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x6:
			strings[0] = "Bridge";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Host bridge";
				strings[2] = "Unused";
				break;
			case 0x1:
				strings[1] = "ISA bridge";
				strings[2] = "Unused";
				break;
			case 0x2:
				strings[1] = "EISA bridge";
				strings[2] = "Unused";
				break;
			case 0x3:
				strings[1] = "MCA bridge";
				strings[2] = "Unused";
				break;
			case 0x4:
				strings[1] = "PCI-to-PCI bridge";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Normal decode";
					break;
				case 0x1:
					strings[2] = "Subtractive decode";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x5:
				strings[1] = "PCMCIA bridge";
				strings[2] = "Unused";
				break;
			case 0x6:
				strings[1] = "NuBus bridge";
				strings[2] = "Unused";
				break;
			case 0x7:
				strings[1] = "CardBus bridge";
				strings[2] = "Unused";
				break;
			case 0x8:
				strings[1] = "RACEway bridge";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Transport mode";
					break;
				case 0x1:
					strings[2] = "Endpoint mode";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x9:
				strings[1] = "PCI-to-PCI bridge";
				switch (progIF)
				{
				case 0x40:
					strings[2] = "Semi-transparent, primary bus towards host CPU";
					break;
				case 0x80:
					strings[2] = "Semi-transparent, secondary bus towards host CPU";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0xa:
				strings[1] = "InfiniBand-to-PCI host bridge";
				strings[2] = "Unused";
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0x7:
			strings[0] = "Simple communication controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Serial controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "8250-compatible (generic XT)";
					break;
				case 0x1:
					strings[2] = "16450-compatible";
					break;
				case 0x2:
					strings[2] = "16550-compatible";
					break;
				case 0x3:
					strings[2] = "16650-compatible";
					break;
				case 0x4:
					strings[2] = "16750-compatible";
					break;
				case 0x5:
					strings[2] = "16850-compatible";
					break;
				case 0x6:
					strings[2] = "16950-compatible";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x1:
				strings[1] = "Parallel controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Standard parallel port";
					break;
				case 0x1:
					strings[2] = "Bi-directional parallel port";
					break;
				case 0x2:
					strings[2] = "ECP 1.X compliant parallel port";
					break;
				case 0x3:
					strings[2] = "IEEE 1284 controller";
					break;
				case 0xfe:
					strings[2] = "IEEE 1284 target device";
					break;
				default:
					strings[2] = "unknown";
				}
				break;
			case 0x2:
				strings[1] = "Multiport serial controller";
				strings[2] = "Unused";
				break;
			case 0x3:
				strings[1] = "Modem";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic modem";
					break;
				case 0x1:
					strings[2] = "Hayes 16450-compatible interface";
					break;
				case 0x2:
					strings[2] = "Hayes 16550-compatible interface";
					break;
				case 0x3:
					strings[2] = "Hayes 16650-compatible interface";
					break;
				case 0x4:
					strings[2] = "Hayes 16750-compatible interface";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x4:
				strings[1] = "IEEE 488.1/2 (GPIB) controller";
				strings[2] = "Unused";
				break;
			case 0x5:
				strings[1] = "Smart card controller";
				strings[2] = "Unused";
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0x8:
			strings[0] = "Base system peripheral";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "PIC";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic 8259-compatible";
					break;
				case 0x1:
					strings[2] = "ISA-compatible";
					break;
				case 0x2:
					strings[2] = "EISA-compatible";
					break;
				case 0x10:
					strings[2] = "I/O APIC interrupt controller";
					break;
				case 0x20:
					strings[2] = "I/O(x) APIC interrupt controller";
					break;
				default:
					strings[2] = "unknown";
				}
				break;
			case 0x1:
				strings[1] = "DMA controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic 8237-compatible";
					break;
				case 0x1:
					strings[2] = "ISA-compatible";
					break;
				case 0x2:
					strings[2] = "EISA-compatible";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x2:
				strings[1] = "Timer";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic 8254-compatible";
					break;
				case 0x1:
					strings[2] = "ISA-compatible";
					break;
				case 0x2:
					strings[2] = "EISA-compatible";
					break;
				case 0x3:
					strings[2] = "HPET";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x3:
				strings[1] = "RTC controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic RTC";
					break;
				case 0x1:
					strings[2] = "ISA-compatible";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x4:
				strings[1] = "PCI hot-plug controller";
				strings[2] = "Unused";
				break;
			case 0x5:
				strings[1] = "SD host controller";
				strings[2] = "Unused";
				break;
			case 0x6:
				strings[1] = "IOMMU";
				break;
				strings[2] = "Unused";
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0x9:
			strings[0] = "Input device controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Keyboard controller";
				strings[2] = "Unused";
				break;
			case 0x1:
				strings[1] = "Digitizer pen";
				strings[2] = "Unused";
				break;
			case 0x2:
				strings[1] = "Mouse controller";
				strings[2] = "Unused";
				break;
			case 0x3:
				strings[1] = "Scanner controller";
				strings[2] = "Unused";
				break;
			case 0x4:
				strings[1] = "Gameport controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic";
					break;
				case 0x10:
					strings[2] = "Extended";
					break;
				default:
					strings[2] = "Unknown";
				}
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0xa:
			strings[0] = "Docking station";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Generic";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0xb:
			strings[0] = "Processor";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "386";
				break;
			case 0x1:
				strings[1] = "486";
				break;
			case 0x2:
				strings[1] = "Pentium";
				break;
			case 0x3:
				strings[1] = "Pentium pro";
				break;
			case 0x10:
				strings[1] = "Alpha";
				break;
			case 0x20:
				strings[1] = "PowerPC";
				break;
			case 0x30:
				strings[1] = "MIPS";
				break;
			case 0x40:
				strings[1] = "Co-processor";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0xc:
			strings[0] = "Serial bus controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "FireWire (IEEE 1394) controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "Generic";
					break;
				case 0x10:
					strings[2] = "OHCI";
					break;
				default:
					strings[2] = "Unkown";
				}
				break;
			case 0x1:
				strings[1] = "ACCESS bus controller";
				strings[2] = "Unused";
				break;
			case 0x2:
				strings[1] = "SSA";
				strings[2] = "Unused";
				break;
			case 0x3:
				strings[1] = "USB controller";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "UHCI controller";
					break;
				case 0x10:
					strings[2] = "OHCI controller";
					break;
				case 0x20:
					strings[2] = "EHCI (USB2) controller";
					break;
				case 0x30:
					strings[2] = "XHCI (USB3) controller";
					break;
				case 0x80:
					strings[2] = "Unspecified";
					break;
				case 0xfe:
					strings[2] = "USB device (not a host controller)";
					break;
				default:
					strings[2] = "Unkown";
				}
				break;
			case 0x4:
				strings[1] = "Fibre channel";
				strings[2] = "Unused";
				break;
			case 0x5:
				strings[1] = "SMBus controller";
				strings[2] = "Unused";
				break;
			case 0x6:
				strings[1] = "InfiniBand controller";
				strings[2] = "Unused";
				break;
			case 0x7:
				strings[1] = "IPMI interface";
				switch (progIF)
				{
				case 0x0:
					strings[2] = "SMIC";
					break;
				case 0x1:
					strings[2] = "Keyboard controller style";
					break;
				case 0x2:
					strings[2] = "Block transfer";
					break;
				default:
					strings[2] = "Unkown";
				}
				break;
			case 0x8:
				strings[1] = "SERCOS interface (IEC 61491)";
				strings[2] = "Unused";
				break;
			case 0x9:
				strings[1] = "CANbus controller";
				strings[2] = "Unused";
				break;
			case 0x80:
				strings[1] = "Other";
				strings[2] = "Unused";
				break;
			default:
				strings[1] = "Unknown";
				strings[2] = "Unknown";
			}
			break;
		case 0xd:
			strings[0] = "Wireless controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "iRDA compatible controller";
				break;
			case 0x1:
				strings[1] = "Consumer IR controller";
				break;
			case 0x10:
				strings[1] = "RF controller";
				break;
			case 0x11:
				strings[1] = "Bluetooth controller";
				break;
			case 0x12:
				strings[1] = "Broadband controller";
				break;
			case 0x20:
				strings[1] = "Ethernet controller (802.1a)";
				break;
			case 0x21:
				strings[1] = "Ethernet controller (802.1b)";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0xe:
			strings[0] = "Intelligent controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "I20";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0xf:
			strings[0] = "Satellite communication controller";
			switch (subclass)
			{
			case 0x1:
				strings[1] = "Satellite TV controller";
				break;
			case 0x2:
				strings[1] = "Satellite audio controller";
				break;
			case 0x3:
				strings[1] = "Satellite voice controller";
				break;
			case 0x4:
				strings[1] = "Satellite data controller";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x10:
			strings[0] = "Encryption controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "Network and computing encryption/decryption";
				break;
			case 0x10:
				strings[1] = "Entertainment encryption/decryption";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x11:
			strings[0] = "Signal processing controller";
			switch (subclass)
			{
			case 0x0:
				strings[1] = "DPIO modules";
				break;
			case 0x1:
				strings[1] = "Performance counters";
				break;
			case 0x10:
				strings[1] = "Communication synchronizer";
				break;
			case 0x20:
				strings[1] = "Signal processing management";
				break;
			case 0x80:
				strings[1] = "Other";
				break;
			default:
				strings[1] = "Unknown";
			}
			strings[2] = "Unused";
			break;
		case 0x12:
			strings[0] = "Processing accelerator";
			strings[1] = "Unused";
			strings[2] = "Unused";
			break;
		case 0x13:
			strings[0] = "Non-essential instrumentation";
			strings[1] = "Unused";
			strings[2] = "Unused";
			break;
		case 0x40:
			strings[0] = "Co-processor";
			strings[1] = "Unused";
			strings[2] = "Unused";
			break;
		case 0xff:
			strings[0] = "Unassigned class (Vendor specific)";
			strings[1] = "Unknown";
			strings[2] = "Unknown";
			break;
		default:
			strings[0] = "Reserved";
			strings[1] = "Unknown";
			strings[2] = "Unknown";
		}
	}

	void InitializeDevices()
	{
		for (word busNr = 0; busNr < 0x100; busNr++)
			for (byte deviceNr = 0; deviceNr < 0x20; deviceNr++)
			{
				if (!deviceExists(busNr, deviceNr, 0))
					continue;
				byte headerType = getHeaderType(busNr, deviceNr, 0);
				byte funcLim = headerType & 0x80 ? 8 : 1;
				headerType &= 0x7f;
				for (byte funcNr = 0; funcNr < funcLim; funcNr++)
					if (deviceExists(busNr, deviceNr, funcNr))
					{
						PCIDevice device;
						device.location = PCILocation(busNr, deviceNr, funcNr);
						device.loadHeader();

						switch (device.header.classCode)
						{
						case (byte)DeviceClass::massStorageController:
							Disk::ControllerDetected(device);
							break;
						case (byte)DeviceClass::multimediaController:
							if (device.header.subclass == (byte)MultimediaController::audioDevice)
							{
								VERBOSE_LOG("Initializing Audio device...\n");
								Audio::DeviceDetected(device);
							}
							break;
						case (byte)DeviceClass::networkController:
							if (device.header.subclass == (byte)NetworkController::ethernetCotnroller)
							{
								VERBOSE_LOG("Initializing Network controller device...\n");
								Network::ControllerDetected(device);
							}
							break;
						default:
							break;
						}
					}
			}
	}

	void EnumerateDevices()
	{
		bool first = true;
		for (word busNr = 0; busNr < 0x100; busNr++)
			for (byte deviceNr = 0; deviceNr < 0x20; deviceNr++)
			{
				if (!deviceExists(busNr, deviceNr, 0))
					continue;
				byte headerType = getHeaderType(busNr, deviceNr, 0);
				byte funcLim = headerType & 0x80 ? 8 : 1;
				headerType &= 0x7f;
				for (byte funcNr = 0; funcNr < funcLim; funcNr++)
					if (deviceExists(busNr, deviceNr, funcNr))
					{
						PCIDevice device;
						device.location = PCILocation(busNr, deviceNr, funcNr);
						device.loadHeader();

						// display device...
						const char *strings[3];
						classSubclassProgIF_toString(device.header.classCode, device.header.subclass, device.header.progIF, strings);
						if (first)
							first = false;
						else
							System::pause(false);
						cout << "Device found on bus " << busNr << ", device " << deviceNr << ", function " << funcNr
							 << ostream::base::hex
							 << " (" << device.header.vendorId << ':' << device.header.deviceId
							 << "):\n    Header type: 0x" << headerType
							 << "\n    Class code: " << strings[0] << " (0x" << device.header.classCode << "),\n"
							 << "    Subclass: " << strings[1] << " (0x" << device.header.subclass << "),\n"
							 << "    ProgIF: " << strings[2] << " (0x" << device.header.progIF << ostream::base::dec << ")\n";

						/*while (!Keyboard::getChar())
							;*/
					}
			}
	}
}