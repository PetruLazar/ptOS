#pragma once
#include <types.h>

namespace Time
{

	void Initialize();

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