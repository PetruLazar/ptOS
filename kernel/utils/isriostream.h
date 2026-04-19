#pragma once

#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include <iostream.h>

namespace std
{
	extern ostream& isrcout;
}

void inline isr_displayMemoryByte(byte *address)
{
	if (*address < 0x10)
		std::isrcout << '0';
	std::isrcout << *address;
}
/*Displays 16 bytes starting at the given memory address*/
void inline isr_displayMemoryRow(byte *block)
{
	std::isrcout << (void *)block << ':' << std::ostream::base::hex;
	for (int i = 0; i < 16; i++)
	{
		std::isrcout << (i == 8 ? "  " : " ");
		isr_displayMemoryByte(block + i);
	}
	std::isrcout << '\n'
				   << std::ostream::base::dec;
}
void inline isr_DisplayMemoryBlock(byte *block, ull len)
{
	for (ull i = 0; i < len; i += 0x10)
		isr_displayMemoryRow(block + i);
}