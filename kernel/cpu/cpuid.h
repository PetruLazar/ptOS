#pragma once
#include <types.h>

inline void cpuid(dword code, dword &eax, dword &ebx, dword &ecx, dword &edx)
{
	asm volatile(
		"cpuid"
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
		: "a"(code));
}
inline void cpuid(dword code, dword arg, dword &eax, dword &ebx, dword &ecx, dword &edx)
{
	asm volatile(
		"cpuid"
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
		: "a"(code), "c"(arg));
}
inline dword read_msr32(dword msr)
{
	dword val;
	asm volatile(
		"rdmsr"
		: "=a"(val)
		: "c"(msr));
	return val;
}
inline qword read_msr64(dword msr)
{
	dword valL, valH;
	asm volatile(
		"rdmsr"
		: "=a"(valL), "=d"(valH)
		: "c"(msr));
	return ((ull)valH << 32) | valL;
}