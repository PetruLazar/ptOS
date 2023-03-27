#pragma once
#include <iostream.h>
#include "../cpu/ports.h"

class PCI
{
	static constexpr word configAddressPort = 0xcf8,
						  configDataPort = 0xcfc;

	struct deviceFunction
	{
		byte unused, progIF;
		byte subclass, classCode;
	};

	static void Initialize();
	static inline dword getConfigReg(byte busNr, byte deviceNr, byte funcNr, byte regOffsest)
	{
		deviceNr &= 0x1f;
		funcNr &= 0x7;
		dword val = (1u << 31) | (dword(busNr) << 16) | (dword(deviceNr) << 11) | (dword(funcNr) << 8) | (regOffsest & 0xfc);
		outdw(configAddressPort, val);
		return indw(configDataPort);
	}

	static inline word getVendorId(byte busNr, byte deviceNr, byte funcNr)
	{
		return getConfigReg(busNr, deviceNr, funcNr, 0) & 0xffff;
	}
	static inline deviceFunction getFunction(byte busNr, byte deviceNr, byte funcNr)
	{
		dword val = getConfigReg(busNr, deviceNr, funcNr, 0x8);
		return *(deviceFunction *)(&val);
	}
	static inline bool deviceExists(byte busNr, byte deviceNr, byte funcNr)
	{
		return getVendorId(busNr, deviceNr, funcNr) != 0xffff;
	}
	static inline byte getHeaderType(byte busNr, byte deviceNr, byte funcNr)
	{
		return getConfigReg(busNr, deviceNr, funcNr, 0xc) >> 16 & 0xff;
	}

public:
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

	static void InitializeDevices();
	static void EnumerateDevices();
};