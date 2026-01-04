#include "pit.h"
#include "idt.h"
#include "../ports.h"

#include <iostream.h>
using namespace std;

namespace PIT
{
	static constexpr word channel_dataport[3] = {0x40, 0x41, 0x42},
						  modeCmdRegister = 0x43;

	static constexpr uint timerFrequency = 1193182; // Hz

	ull timeSeconds = 0;
	uint partialSecondCounter = 0;
	uint timerCounter;

	enum class AccessMode
	{
		latchCountValueCommand,
		lobyte,
		hibyte,
		lobyte_hibyte
	};

	void ConfigureChannel(SelectChannel channel, OperatingMode opMode, uint desiredFrequency)
	{
		// get counter value
		uint targetCounter = timerFrequency / desiredFrequency;
		if (timerFrequency % desiredFrequency >= (desiredFrequency >> 1))
			targetCounter++;
		if (targetCounter > 0x10000)
			targetCounter = 0x10000;

		outb(modeCmdRegister, (byte)channel << 6 | ((byte)AccessMode::lobyte_hibyte << 4) | ((byte)opMode << 1));

		timerCounter = targetCounter;
		outb(channel_dataport[(byte)channel], targetCounter & 0xff); // low byte
		outb(channel_dataport[(byte)channel], targetCounter >> 8);   // high byte
	}

	void InterruptHandler(registers_t &regs)
	{
		partialSecondCounter += timerCounter;
		while (partialSecondCounter > timerFrequency)
		{
			timeSeconds++;
			partialSecondCounter -= timerFrequency;
		}
	}
	ull driver_time()
	{
		return timeSeconds * 1000 + (partialSecondCounter * 1000 / timerFrequency);
	}
}