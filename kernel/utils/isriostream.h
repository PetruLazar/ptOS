#pragma once

#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include <iostream.h>

namespace ISR
{
	namespace std
	{
		class istream;
		class ostream;

		extern istream cin;
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

		class istream
		{
			byte flags;

		public:
			// input functions
			inline istream &operator>>(char &ch) // read single char
			{
				do
				{
					ch = Keyboard::driver_getKeyPressedEvent().getChar();
				} while (ch == 0);
				return *this;
			}

			inline istream &operator>>(char str[]) // read line
			{
				ull pos = 0;
				while (true)
				{
					char ch;
					operator>>(ch);
					switch (ch)
					{
					case '\b': // delete one char
						if (!pos)
							break;
						cout << "\b \b";
						pos--;
						break;
					case '\r': // finish reading
						str[pos] = 0;
						cout << '\n';
						return *this;
					case 0:
						break;
					default:
						cout << ch;
						str[pos++] = ch;
					}
				}
			}

			inline istream &operator>>(uint &nr) // read unsigned
			{
				ull pos = 0;
				nr = 0;
				while (true)
				{
					char ch;
					operator>>(ch);
					switch (ch)
					{
					case '\b':
						if (!pos)
							break;
						nr /= 10;
						pos--;
						cout << "\b \b";
						break;
					case '\r':
						if (pos)
						{
							cout << '\n';
							return *this;
						}
						break;
					case 0:
						break;
					default:
						if (ch >= '0' && ch <= '9')
						{
							pos++;
							nr = nr * 10 + (ch - '0');
							cout << ch;
						}
					}
				}
			}
			// inline istream &operator>>(int &nr) // read signed
			// {
			// }
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
		std::cout << (i == 8 ? "  " : " ");
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