#pragma once
#include "types.h"
// #include "vector.h"
// #include "iostream.h"

inline char digit(int v) { return v < 10 ? v + '0' : v - 10 + 'A'; }
inline ull sign(char *&str, llong value)
{
	if (value >= 0)
		return (ull)value;
	*(str++) = '-';
	if (value == 1ull << 63)
		return 1ull << 63;
	return (ull)-value;
}

inline void ulltos2(char *str, uint64_t value)
{
	byte digits = 0;
	if (!value)
		digits = 1;
	else
		for (uint64_t i = value; i; i >>= 2)
			digits++;

	str += digits;
	*str = 0;
	for (; digits; digits--, value >>= 2)
		*--str = digit(value & 1);
}
inline void lltos2(char *str, int64_t value) { ulltos2(str, sign(str, value)); }
inline void ulltos8(char *str, uint64_t value)
{
	byte digits = 0;
	if (!value)
		digits = 1;
	else
		for (uint64_t i = value; i; i >>= 3)
			digits++;

	str += digits;
	*str = 0;
	for (; digits; digits--, value >>= 3)
		*--str = digit(value & 7);
}
inline void lltos8(char *str, int64_t value) { ulltos8(str, sign(str, value)); }
inline void ulltos16(char *str, uint64_t value)
{
	byte digits = 0;
	if (!value)
		digits = 1;
	else
		for (uint64_t i = value; i; i >>= 4)
			digits++;

	str += digits;
	*str = 0;
	for (; digits; digits--, value >>= 4)
		*--str = digit(value & 15);
}
inline void lltos16(char *str, int64_t value) { ulltos16(str, sign(str, value)); }
inline void ulltos(char *str, uint64_t value, int base)
{
	byte digits = 0;
	if (!value)
		digits = 1;
	else
		for (uint64_t i = value; i; i /= base)
			digits++;

	str += digits;
	*str = 0;
	for (; digits; digits--, value /= base)
		*--str = digit(value % base);
}
inline void lltos(char *str, int64_t value, int base) { ulltos(str, sign(str, value), base); }
inline void ulltos(char *str, uint64_t value) { ulltos(str, value, 10); }
inline void lltos(char *str, int64_t value) { ulltos(str, sign(str, value), 10); }

inline void byteToBinString(char *str, byte value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = '0' + (value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
inline void byteToHexString(char *str, byte value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = digit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

inline void wordToBinString(char *str, word value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = '0' + (value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
inline void wordToHexString(char *str, word value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = digit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

inline void dwordToBinString(char *str, dword value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = '0' + (value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
inline void dwordToHexString(char *str, dword value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = digit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

inline void qwordToBinString(char *str, qword value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = '0' + (value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
inline void qwordToHexString(char *str, qword value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = digit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

inline ull strlen(const char *str)
{
	ull len = 0;
	for (; *str; str++)
		len++;
	return len;
}
inline const char *strchr(const char *str, char ch)
{
	while (*str)
	{
		if (*str == ch)
			return str;
		str++;
	}
	return nullptr;
}
inline char *strchr(char *str, char ch)
{
	while (*str)
	{
		if (*str == ch)
			return str;
		str++;
	}
	return nullptr;
}

inline char toLower(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A' + 'a';
	return ch;
}
inline char toUpper(char ch)
{
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 'A';
	return ch;
}