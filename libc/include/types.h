#pragma once

typedef char sbyte, int8_t;
typedef short sword, int16_t;
typedef int sdword, int32_t;
typedef long long sqword, llong, int64_t;

typedef unsigned char byte, uint8_t;
typedef unsigned short word, ushort, uint16_t;
typedef unsigned int dword, uint, uint32_t;
typedef unsigned long long qword, ullong, ull, uint64_t;

#ifndef SKIP_SIZE_T
typedef long unsigned size_t;
#endif

template <typename T>
class UnalignedField
{
	byte data[sizeof(T)];

public:
	inline UnalignedField() {}
	inline UnalignedField(T val) { *this = val; }

	inline T &operator=(T val)
	{
		T &t = *this;
		return t = val;
	}
	inline operator T &() { return *(T *)this; }
};