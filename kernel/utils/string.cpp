#include "string.h"

inline char hexDigit(byte value)
{
	if (value < 10)
		return '0' + value;
	return 'A' - 10 + value;
}

void toString_rec(char *&str, ull value)
{
	if (value >= 10)
		toString_rec(str, value / 10);
	(str++)[0] = '0' + value % 10;
}
void toString_rec(char *&str, ull value, int base)
{
	if (value >= base)
		toString_rec(str, value / base, base);
	(str++)[0] = hexDigit(value % base);
}
void toString2_rec(char *&str, ull value)
{
	if (value >= 2)
		toString2_rec(str, value >> 1);
	(str++)[0] = '0' + (value & 1);
}
void toString8_rec(char *&str, ull value)
{
	if (value >= 8)
		toString8_rec(str, value >> 3);
	(str++)[0] = '0' + value & 7;
}
void toString16_rec(char *&str, ull value)
{
	if (value >= 16)
		toString16_rec(str, value >> 4);
	(str++)[0] = hexDigit(value & 0xf);
}

void ulltos(char *str, uint64_t value)
{
	toString_rec(str, value);
	str[0] = 0;
}
void lltos(char *str, int64_t value)
{
	ull uvalue;
	if (value < 0)
	{
		str[0] = '-';
		str++;
		if (value == 1ull << 63)
			uvalue = 1ull << 63;
		else
			uvalue = -value;
	}
	else
		uvalue = value;
	ulltos(str, uvalue);
}

void ulltos2(char *str, uint64_t value)
{
	toString2_rec(str, value);
	str[0] = 0;
}
void lltos2(char *str, int64_t value)
{
	ull uvalue;
	if (value < 0)
	{
		(str++)[0] = '-';
		if (value == 1ull << 63)
			uvalue = 1ull << 63;
		else
			uvalue = -value;
	}
	else
		uvalue = value;
	ulltos2(str, uvalue);
}

void ulltos8(char *str, uint64_t value)
{
	toString8_rec(str, value);
	str[0] = 0;
}
void lltos8(char *str, int64_t value)
{
	ull uvalue;
	if (value < 0)
	{
		(str++)[0] = '-';
		if (value == 1ull << 63)
			uvalue = 1ull << 63;
		else
			uvalue = -value;
	}
	else
		uvalue = value;
	ulltos8(str, uvalue);
}

void ulltos16(char *str, uint64_t value)
{
	toString16_rec(str, value);
	str[0] = 0;
}
void lltos16(char *str, int64_t value)
{
	ull uvalue;
	if (value < 0)
	{
		(str++)[0] = '-';
		if (value == 1ull << 63)
			uvalue = 1ull << 63;
		else
			uvalue = -value;
	}
	else
		uvalue = value;
	ulltos16(str, uvalue);
}

void ulltos(char *str, uint64_t value, int base)
{
	toString_rec(str, value, base);
	str[0] = 0;
}
void lltos(char *str, int64_t value, int base)
{
	ull uvalue;
	if (value < 0)
	{
		(str++)[0] = '-';
		if (value == 1ull << 63)
			uvalue = 1ull << 63;
		else
			uvalue = -value;
	}
	else
		uvalue = value;
	ulltos(str, uvalue, base);
}

void byteToBinString(char *str, byte value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
void byteToHexString(char *str, byte value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

void wordToBinString(char *str, word value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
void wordToHexString(char *str, word value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

void dwordToBinString(char *str, dword value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
void dwordToHexString(char *str, dword value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}

void qwordToBinString(char *str, qword value)
{
	for (int i = 8 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0x1);
		value >>= 1;
	}
	str[8 * sizeof(value)] = 0;
}
void qwordToHexString(char *str, qword value)
{
	for (int i = 2 * sizeof(value) - 1; i >= 0; i--)
	{
		str[i] = hexDigit(value & 0xf);
		value >>= 4;
	}
	str[2 * sizeof(value)] = 0;
}
