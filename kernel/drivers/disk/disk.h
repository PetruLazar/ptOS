#pragma once
#include <types.h>
#include <string.h>
#include "../pci.h"

namespace Disk
{
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

	enum class result : word
	{
		success,
		deviceFault,
		errorRegisterSet,
		unknownError,

		deviceNotSupported
	};
	inline std::string resultAsString(result res)
	{
		switch ((result)((word)res & 0xff))
		{
		case result::success:
			return "success";
		case result::deviceFault:
			return "device fault";
		case result::errorRegisterSet:
		{
			word errorReg = (word)res >> 8;
			std::string ret = "";
			if (errorReg & (byte)ATAerror::badBlock)
				ret += "bad block | ";
			if (errorReg & (byte)ATAerror::commandAborted)
				ret += "command aborted | ";
			if (errorReg & (byte)ATAerror::idMarkNotFound)
				ret += "id mark not found | ";
			if (errorReg & (byte)ATAerror::mediaChanged)
				ret += "media changed | ";
			if (errorReg & (byte)ATAerror::mediaChangeRequest)
				ret += "media change request | ";
			if (errorReg & (byte)ATAerror::noAddressMark)
				ret += "no address mark | ";
			if (errorReg & (byte)ATAerror::track0NotFound)
				ret += "track 0 not found | ";
			if (errorReg & (byte)ATAerror::uncorrectableData)
				ret += "uncorrectable data | ";

			if (ret.length() > 0)
				ret.erase(ret.length() - 3, 3);
			return ret;
		}
		case result::unknownError:
			return "unknown error";
		case result::deviceNotSupported:
			return "device not supported";
		}
	}

	class StorageDevice
	{
	public:
		virtual int getSize() = 0;

		virtual result read(uint lba, uint numsec, byte *buffer) = 0;
		virtual result write(uint lba, uint numsec, byte *buffer) = 0;
	};

	extern std::vector<StorageDevice *> *devices;

	void Initialize();
	void CleanUp();
	void ControllerDetected(PCI::DeviceHeader *header);
}