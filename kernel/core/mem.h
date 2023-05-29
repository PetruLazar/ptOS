#pragma once
#include <mem.h>
#include <string.h>

// #define ALLOC_DBG_MSG

namespace Memory
{
	void DisplayMap();
	std::string getStringMemoryMap();
	void Initialize();
}

extern "C" void memcpy(void *dest, const void *src, ull len);
extern "C" void memmove(void *dest, const void *src, ull len);
extern "C" void memset(void *ptr, ull len, byte val);