#pragma once
#include <iostream.h>
#include <string.h>
#include "../cpu/ports.h"

namespace PCI
{
	static constexpr word configAddressPort = 0xcf8,
						  configDataPort = 0xcfc;

	inline dword getConfigReg(byte busNr, byte deviceNr, byte funcNr, byte regOffsest)
	{
		deviceNr &= 0x1f;
		funcNr &= 0x7;
		dword val = (1u << 31) | (dword(busNr) << 16) | (dword(deviceNr) << 11) | (dword(funcNr) << 8) | (regOffsest & 0xfc);
		outdw(configAddressPort, val);
		return indw(configDataPort);
	}
	inline void setConfigReg(byte busNr, byte deviceNr, byte funcNr, byte regOffsest, dword newValue)
	{
		deviceNr &= 0x1f;
		funcNr &= 0x7;
		dword val = (1u << 31) | (dword(busNr) << 16) | (dword(deviceNr) << 11) | (dword(funcNr) << 8) | (regOffsest & 0xfc);
		outdw(configAddressPort, val);
		outdw(configDataPort, newValue);
	}

	struct deviceFunction
	{
		byte revisionID, progIF;
		byte subclass, classCode;
	};
	struct PCILocation
	{
		word busNr;
		byte deviceNr, funcNr;

		inline PCILocation() {}
		inline PCILocation(word busNr, byte deviceNr, byte funcNr) : busNr(busNr), deviceNr(deviceNr), funcNr(funcNr) {}

		inline std::string to_string()
		{
			return "PCI bus " + std::to_string(busNr) + ", device " + std::to_string(deviceNr) + ", function " + std::to_string(funcNr);
		}
	};

	// header with type 0x0
	struct PCIDeviceHeader
	{
		word vendorId, deviceId,
			command, status;
		byte revisionId, progIF, subclass, classCode,
			cacheLineSize, latencyTimer, headerType, BIST;
		uint bar0,
			bar1,
			bar2,
			bar3,
			bar4,
			bar5,
			cardbusCISpointer;
		word subsystemVendorId, subsystemId;
		uint expansionROMbaseAddress;
		byte capabilitiesPointer, reserved1[3];
		uint reserved;
		byte interruptLine, interruptPin, minGrant, maxLatency;
	};

	struct PCIDevice
	{
		PCILocation location;
		PCIDeviceHeader header;

		inline void flushHeader()
		{
			dword *rawHeader = (dword *)&header;
			for (dword i = 0; i < 0x10; i++)
				setConfigReg(location.busNr, location.deviceNr, location.funcNr, i << 2, rawHeader[i]);
		}
		inline void loadHeader()
		{
			dword *rawHeader = (dword *)&header;
			for (dword i = 0; i < 0x10; i++)
				rawHeader[i] = getConfigReg(location.busNr, location.deviceNr, location.funcNr, i << 2);
		}
	};

	void Initialize();
	// inline void setConfigReg(byte busNr, byte deviceNr, byte funcNr, byte regOffset, uint val)

	inline word getVendorId(byte busNr, byte deviceNr, byte funcNr)
	{
		return getConfigReg(busNr, deviceNr, funcNr, 0) & 0xffff;
	}
	inline bool deviceExists(byte busNr, byte deviceNr, byte funcNr)
	{
		return getVendorId(busNr, deviceNr, funcNr) != 0xffff;
	}
	inline byte getHeaderType(byte busNr, byte deviceNr, byte funcNr)
	{
		return getConfigReg(busNr, deviceNr, funcNr, 0xc) >> 16 & 0xff;
	}

	enum class DeviceClass : byte
	{
		unclassified = 0x0,
		massStorageController = 0x1,
		networkController = 0x2,
		displayController = 0x3,
		multimediaController = 0x4,
		memoryController = 0x5,
		bridge = 0x6,
		simpleCommunicationController = 0x7,
		baseSystemPeripheral = 0x8,
		inputDeviceController = 0x9,
		dockingStation = 0xa,
		processor = 0xb,
		serialBusController = 0xc,
		wirelessControler = 0xd,
		intelligentController = 0xe,
		satelliteCommunicationController = 0xf,
		encryptionController = 0x10,
		signalProcessingController = 0x11,
		processingAccelerator = 0x12,
		nonEssentialInstrumentation = 0x13,
		coProcessor = 0x40,
		unassigned = 0xff
	};

	enum class UnclassifiedSubclass : byte
	{
		nonVGAcompatible = 0x0,
		VGAcompatible = 0x1,
	};
	enum class MassStorageController : byte
	{
		SCSIbusController = 0x0,
		IDEController = 0x1,
		floppyDiskController = 0x2,
		IPIbusController = 0x3,
		RAIDcontroller = 0x4,
		ATAcontroller = 0x5,
		SATAcontroller = 0x6,
		serialAttachedSCSIcontroller = 0x7,
		nonVolatileMemoryController = 0x8,

		other = 0x80,
	};
	enum class MultimediaController : byte
	{
		videoController = 0x0,
		audioController = 0x1,
		computerTelephonyDevice = 0x2,
		audioDevice = 0x3,

		other = 0x80
	};
	enum class NetworkController : byte
	{
		ethernetCotnroller = 0x0,
		tokenRingController = 0x1,
		FDDIcontroller = 0x2,
		ATMcontroller = 0x3,
		ISDNcontroller = 0x4,
		worldFlipController = 0x5,
		PICMG2_14multiComputingController = 0x6,
		inifibandController = 0x7,
		fabricController = 0x8,

		other = 0x80,
	};

	void InitializeDevices();
	void EnumerateDevices();
};