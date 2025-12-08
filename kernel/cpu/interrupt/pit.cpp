#include "pit.h"
#include "idt.h"
#include "../ports.h"

#include <iostream.h>
using namespace std;

namespace PIT
{
	static constexpr word channel0_dataport = 0x40,
						  channel1_dataport = 0x41,
						  channel2_dataport = 0x42,
						  modeCmdRegister = 0x43;

	static constexpr uint frequency = 1193182;

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
		uint targetCounter = frequency / desiredFrequency;
		if (frequency % desiredFrequency >= (desiredFrequency >> 1))
			targetCounter++;
		if (targetCounter > 0x10000)
			targetCounter = 0x10000;

		outb(modeCmdRegister, (byte)channel << 6 | ((byte)AccessMode::lobyte_hibyte << 4) | ((byte)opMode << 1));

		outb(channel0_dataport + (byte)channel, targetCounter & 0xff); // low byte
		outb(channel0_dataport + (byte)channel, targetCounter >> 8);   // high byte
	}
}