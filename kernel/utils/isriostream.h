#pragma once

#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include <iostream.h>

namespace ISR
{
	namespace std
	{
		class ostream;

		extern ostream cout;

		class ostream
		{
		private:
			// bits 0 - 2 : base
			byte flags = 0b10;

			::std::ostream::base getCurrBase()
			{
				return (::std::ostream::base)(flags & 0b11);
			}

		public:
			inline ostream &operator<<(const char *str)
			{
				Screen::driver_print(str);
				return *this;
			}
			inline ostream &operator<<(void *ptr)
			{
				char str[17];
				qwordToHexString(str, (qword)ptr);
				Screen::driver_print(str);
				return *this;
			}
			inline ostream &operator<<(char character)
			{
				Screen::driver_print(character);
				return *this;
			}
			inline ostream &operator<<(short val) { return operator<<((llong)val); }
			inline ostream &operator<<(ushort val) { return operator<<((ullong)val); }
			inline ostream &operator<<(int val) { return operator<<((llong)val); }
			inline ostream &operator<<(uint val) { return operator<<((ullong)val); }
			inline ostream &operator<<(long val) { return operator<<((llong)val); }
			inline ostream &operator<<(unsigned long val) { return operator<<((ullong)val); }
			inline ostream &operator<<(llong val)
			{
				switch (getCurrBase())
				{
				case ::std::ostream::base::hex:
				{
					char str[20];
					lltos16(str, val);
					Screen::driver_print(str);
					return *this;
				}
				case ::std::ostream::base::oct:
				{
					char str[32];
					lltos8(str, val);
					Screen::driver_print(str);
					return *this;
				}
				case ::std::ostream::base::bin:
				{
					char str[65];
					lltos2(str, val);
					Screen::driver_print(str);
					return *this;
				}
				}
				char str[32];
				lltos(str, val);
				Screen::driver_print(str);
				return *this;
			}
			inline ostream &operator<<(ullong val)
			{
				switch (getCurrBase())
				{
				case ::std::ostream::base::hex:
				{
					char str[20];
					ulltos16(str, val);
					Screen::driver_print(str);
					return *this;
				}
				case ::std::ostream::base::oct:
				{
					char str[32];
					ulltos8(str, val);
					Screen::driver_print(str);
					return *this;
				}
				case ::std::ostream::base::bin:
				{
					char str[65];
					ulltos2(str, val);
					Screen::driver_print(str);
					return *this;
				}
				}
				char str[32];
				ulltos(str, val);
				Screen::driver_print(str);
				return *this;
			}
			inline ostream &operator<<(::std::ostream::base base)
			{
				flags &= ~0b11;
				flags |= (byte)base & 0b11;
				return *this;
			}
		};
	}
}

void inline isr_displayMemoryByte(byte *address)
{
	if (*address < 0x10)
		ISR::std::cout << '0';
	ISR::std::cout << *address;
}
/*Displays 16 bytes starting at the given memory address*/
void inline isr_displayMemoryRow(byte *block)
{
	ISR::std::cout << (void *)block << ':' << std::ostream::base::hex;
	for (int i = 0; i < 16; i++)
	{
		ISR::std::cout << (i == 8 ? "  " : " ");
		isr_displayMemoryByte(block + i);
	}
	ISR::std::cout << '\n'
				   << std::ostream::base::dec;
}
void inline isr_DisplyMemoryBlock(byte *block, ull len)
{
	for (ull i = 0; i < len; i += 0x10)
		isr_displayMemoryRow(block + i);
}