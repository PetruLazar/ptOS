#include "apic.h"
#include "../../core/paging.h"
#include "../../core/mem.h"
#include "../../core/sys.h"
#include "../../utils/isriostream.h"
#include "pit.h"
#include "irq.h"
#include "../../utils/time.h"

namespace APIC
{
	class LocalAPIC
	{
	public:
		class Register
		{
		public:
			dword value;
		private:
			byte reserved[0xc];
		};

	private:
		Register reserved1[2];

	public:
		Register localAPICID;
		Register localAPICRVersion;

	private:
		Register reserved2[4];

	public:
		Register taskPriorityRegister;
		Register arbitrationProrityRegister;
		Register processorPriorityRegister;
		Register EOIRegister;
		Register remoteReadRegister;
		Register logicalDestinationRegister;
		Register destinationFormatRegister;
		Register spuriousInterruptVectorRegister;
		Register inServiceRegister[8];
		Register triggerModeRegister[8];
		Register interruptRequestRegister[8];
		Register errorStatusRegister;

	private:
		Register reserved3[6];

	public:
		Register LVT_CMCI;
		Register interruptCommandRegister[2];
		Register LVT_timerRegister;
		Register LVT_thermalSensorRegister;
		Register LVT_performanceMonitoringCountersRegister;
		Register LVT_LINT0Register;
		Register LVT_LINT1Register;
		Register LVT_errorRegister;
		Register initialTimerCountRegister;
		Register currentTimerCountRegister;

	private:
		Register reserved4[4];

	public:
		Register divideTimerConfigurationRegister;

	private:
		Register reserved5[1];

	};

	volatile LocalAPIC* localAPIC;

	void Initialize()
	{
		// get APIC base
		localAPIC = (volatile LocalAPIC*)(read_msr64(0x1b) & PageEntry::addressMask_4kb);

		// map APIC registers
		PageMapLevel4 &current = PageMapLevel4::getCurrent();
		void *pageSpace;
		dword *pageAllocationMap;
		Memory::GetPageSpace(pageSpace, pageAllocationMap);
		if (!current.mapRegion(pageSpace, *pageAllocationMap, (qword)localAPIC, (qword)localAPIC, 0x1000, PageEntry::EntryAttributes(PageEntry::writeAccessBit | PageEntry::pageWriteThroughBit | PageEntry::pageCacheDisable)))
		{
			// handle error case
			System::blueScreen();
		}

		// temporarily fall back to legacy PIC so code can be developed further
		PIT::ConfigureChannel(PIT::SelectChannel::channel0, PIT::OperatingMode::rateGenerator, 1000 / IRQ::ms_per_timeint);
		Time::SelectTimer(Time::TimerSource::PIT);
	}
}