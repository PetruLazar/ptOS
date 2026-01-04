#pragma once
#include "../cpuid.h"

namespace APIC
{
	inline bool DetectPresence()
	{
		dword unused, edx;
		cpuid(1, unused, unused, unused, edx);
		return (edx >> 9) & 0x1;
	}

	void Initialize();
}