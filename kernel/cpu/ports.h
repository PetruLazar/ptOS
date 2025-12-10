#pragma once
#include <types.h>

inline byte inb(word port)
{
	byte data;
	asm volatile(
		"in %[data], %[port]"
		: [data] "=a"(data)
		: [port] "d"(port));
	return data;
}
inline word inw(word port)
{
	word data;
	asm volatile(
		"in %[data], %[port]"
		: [data] "=a"(data)
		: [port] "d"(port));
	return data;
}
inline dword indw(word port)
{
	dword data;
	asm volatile(
		"in %[data], %[port]"
		: [data] "=a"(data)
		: [port] "d"(port));
	return data;
}
inline void outb(word port, byte data)
{
	asm volatile(
		"out %[port], %[data]"
		:
		: [port] "d"(port), [data] "a"(data));
}
inline void outw(word port, word data)
{
	asm volatile(
		"out %[port], %[data]"
		:
		: [port] "d"(port), [data] "a"(data));
}
inline void outdw(word port, dword data)
{
	asm volatile(
		"out %[port], %[data]"
		:
		: [port] "d"(port), [data] "a"(data));
}

void io_wait();