#pragma once
#include "acpi.h"

namespace ACPI
{
	class GenericAddressStructure
	{
	public:
		enum class AddressSpaceID : byte
		{
			systemMemorySpace = 0,
			systemIOSpace,
			pciConfigurationSpace,
			embeddedController,
			smBus,
			systemCmos,
			pciBarTarget,
			ipmi,
			generalPurposeIO,
			genericSerialBus,
			platformCommunicationsChannel,
			platformRuntimeMechanism,

			functionalFixedHardware = 0x7f,
		};
		enum class AccessSize : byte
		{
			undefined = 0,
			byteAccess,
			wordAccess,
			dwordAccess,
			qwordAccess,
		};

		AddressSpaceID addressSpaceId;
		byte bitWidth;
		byte bitOffset;
		AccessSize accessSize;
		UnalignedField<void*> address;
	};

	class FADT : public GenericSDT
	{
	public:
		enum class PMProfile : byte
		{
			unspecified = 0,
			desktop,
			mobile,
			workstation,
			enterpriseServer,
			SOHOserver,
			appliancePC,
			performanceServer,
			tablet,
		};
		class FixedFlags
		{
		public:
			dword wbinvd : 1;
			dword wbinvdFlush : 1;
			dword procC1 : 1;
			dword plvl2UP : 1;
			dword pwrButton : 1;
			dword slpButton : 1;
			dword fixRtc : 1;
			dword rtcS4 : 1;
			dword tmrValExt : 1;
			dword dckCap : 1;
			dword resetRegSup : 1;
			dword sealedCase : 1;
			dword headless : 1;
			dword cpuSwSlp : 1;
			dword pciExpWak : 1;
			dword usePlatformClock : 1;
			dword s4RtcStsValid : 1;
			dword remotePowerOnCapable : 1;
			dword forceApicClusterModel : 1;
			dword forceApicPhysicalDestinationMode : 1;
			dword hwReducedAcpi : 1;
			dword lowPowerS0IdleCapable : 1;
			dword persistentCpuCaches : 1;
		};

		dword firmwareCtrl;
		dword dsdt;
		byte reserved1;
		PMProfile preferredPMProfile;
		word sciInt;
		dword smiCmd;
		byte acpiEnable;
		byte acpiDisable;
		byte s4biosReq;
		byte pstateCnt;
		dword pm1aEvtBlk;
		dword pm1bEvtBlk;
		dword pm1aCntBlk;
		dword pm1bCntBlk;
		dword pm2CntBlk;
		dword pmTmrBlk;
		dword gpe0Blk;
		dword gpe1Blk;
		byte pm1EvtLen;
		byte pm1CntLen;
		byte pm2CntLen;
		byte pmTmrLen;
		byte gpe0BlkLen;
		byte gpe1BlkLen;
		byte gpe1Base;
		byte cstCnt;
		word pLvl2Lat;
		word pLvl3Lat;
		word flushSize;
		word flushStride;
		byte dutyOffset;
		byte dutyWidth;
		byte day_alrm;
		byte mon_alrm;
		byte century;
		UnalignedField<word> bootArchFlags_iapc;
		byte reserved2;
		FixedFlags flags;
		GenericAddressStructure resetReg;
		byte resetValue;
		UnalignedField<word> bootArchFlags_arm;
		byte minorFadtVersion;
		UnalignedField<void*> xFirmwareCtrl;
		UnalignedField<void*> x_dsdt;
		GenericAddressStructure x_pm1aEvtBlk;
		GenericAddressStructure x_pm1bEvtBlk;
		GenericAddressStructure x_pm1aCntBlk;
		GenericAddressStructure x_pm1bCntBlk;
		GenericAddressStructure x_pm2CntBlk;
		GenericAddressStructure x_pmTmrBlk;
		GenericAddressStructure x_gpe0Blk;
		GenericAddressStructure x_gpe1Blk;
		GenericAddressStructure sleepControlReg;
		GenericAddressStructure sleepStatusReg;
		const char hypervisorVendorId[8];
	};

	void InitializeFADT();
	volatile FADT* GetFADT();
}