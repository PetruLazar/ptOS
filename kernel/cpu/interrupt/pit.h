#pragma once
#include <types.h>

namespace PIT
{
	enum class SelectChannel
	{
		channel0,
		channel1,
		channel2,
		readBack
	};
	enum class OperatingMode
	{
		oneShot,
		mode1, // similar to oneShot
		rateGenerator,
		squareWaveGenerator,
		softTriggeredStrobe,
		hardTriggeredStrobe,
		rateGenerator2,		  // same as rateGenerator
		squareWaveGenerator2, // same as squareWaveGenerator
	};

	void ConfigureChannel(SelectChannel channel, OperatingMode opMode, uint desiredFrequency);
}