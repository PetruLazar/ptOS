#pragma once
#include <keyboard.h>
#include <screen.h>
#include <cstring.h>

namespace std
{
	class istream;
	class ostream;

	extern istream cin;
	extern ostream& cout;
	extern ostream& nullout;

	class ostream
	{
	public:
		enum class base : byte
		{
			bin,
			oct,
			dec,
			hex
		};

	private:
		// bits 0 - 2 : base
		byte flags = 0b10;

		base getCurrBase()
		{
			return (base)(flags & 0b11);
		}

	protected:
		virtual void write(const byte* buffer, ull len) = 0;

	public:
		inline ostream &operator<<(const char *str)
		{
			write((const byte*)str, strlen(str));
			return *this;
		}
		inline ostream &operator<<(void *ptr)
		{
			char str[17];
			qwordToHexString(str, (qword)ptr);
			return *this << str;
		}
		inline ostream &operator<<(char character)
		{
			write((const byte*)&character, 1);
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
			case base::hex:
			{
				char str[20];
				lltos16(str, val);
				return *this << str;
			}
			case base::oct:
			{
				char str[32];
				lltos8(str, val);
				return *this << str;
			}
			case base::bin:
			{
				char str[65];
				lltos2(str, val);
				return *this << str;
			}
			}
			char str[32];
			lltos(str, val);
			return *this << str;
		}
		inline ostream &operator<<(ullong val)
		{
			switch (getCurrBase())
			{
			case base::hex:
			{
				char str[20];
				ulltos16(str, val);
				return *this << str;
			}
			case base::oct:
			{
				char str[32];
				ulltos8(str, val);
				return *this << str;
			}
			case base::bin:
			{
				char str[65];
				ulltos2(str, val);
				return *this << str;
			}
			}
			char str[32];
			ulltos(str, val);
			return *this << str;
		}
		inline ostream &operator<<(base base)
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
				ch = Keyboard::getKeyPressedEvent().getChar();
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

void inline displayMemoryByte(const byte *address)
{
	if (*address < 0x10)
		std::cout << '0';
	std::cout << *address;
}
/*Displays 16 bytes starting at the given memory address*/
void inline displayMemoryRow(const byte *block)
{
	std::cout << (void *)block << ':' << std::ostream::base::hex;
	for (int i = 0; i < 16; i++)
	{
		std::cout << (i == 8 ? "  " : " ");
		displayMemoryByte(block + i);
	}
	std::cout << '\n'
			  << std::ostream::base::dec;
}
void inline DisplayMemoryBlock(const byte *block, ull len)
{
	for (ull i = 0; i < len; i += 0x10)
		displayMemoryRow(block + i);
}