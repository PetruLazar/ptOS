#pragma once
#include <types.h>

namespace Time
{
	enum class TimerSource
	{
		PIT,
		APICtimer,
		HPET,

		noTimer
	};

	void Initialize();
	void SelectTimer(TimerSource timerSource);

	inline qword clock()
	{
		dword retValH, retValL;
		asm volatile(
			"rdtsc"
			: "=a"(retValL), "=d"(retValH));
		return ((qword)retValH << 32) | retValL;
	}
	qword driver_time();
}