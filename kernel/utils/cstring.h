#pragma once
#include "types.h"
#include "vector.h"
#include "iostream.h"

void ulltos(char *str, uint64_t value);
void lltos(char *str, int64_t value);
void ulltos2(char *str, uint64_t value);
void lltos2(char *str, int64_t value);
void ulltos8(char *str, uint64_t value);
void lltos8(char *str, int64_t value);
void ulltos16(char *str, uint64_t value);
void lltos16(char *str, int64_t value);
void ulltos(char *str, uint64_t value, int base);
void lltos(char *str, int64_t value, int base);

void byteToBinString(char *str, byte value);
void byteToHexString(char *str, byte value);

void wordToBinString(char *str, word value);
void wordToHexString(char *str, word value);

void dwordToBinString(char *str, dword value);
void dwordToHexString(char *str, dword value);

void qwordToBinString(char *str, qword value);
void qwordToHexString(char *str, qword value);

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