#pragma once
#include "../utils/types.h"

inline void cpuid(dword code, dword &eax, dword &ebx, dword &ecx, dword &edx)
{
	asm("cpuid"
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
		: "a"(code));
}