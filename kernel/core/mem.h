#pragma once
#include <mem.h>
#include <string.h>

// #define ALLOC_DBG_MSG

namespace Memory
{
	void DisplayMap();
	std::string getStringMemoryMap();
	void Initialize(byte *kernelPhysicalAddress, byte *mapEntryDescriptor, byte *mapEntries);

	void GetPageSpace(void *&pageSpace, dword &pageAllocationMap);
}

inline void memcpy(void *dest, const void *src, ull len)
{
	asm volatile(
		"cld\n"
		"rep movsb"
		: "=S"(src), "=D"(dest), "=c"(len)
		: "S"(src), "D"(dest), "c"(len)
	);
}
extern "C" void memmove(void *dest, const void *src, ull len);
inline void memset(void *ptr, ull len, byte val)
{
	asm volatile(
		"cld\n"
		"rep stosb"
		: "=D"(ptr), "=c"(len)
		: "D"(ptr), "c"(len), "a"(val)
	);
}