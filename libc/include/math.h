#pragma once
#include <types.h>

// return the smallest multiple of 'alignment', grater or equal to value
inline ull alignValueUpwards(ull value, ull alignment)
{
	ull rem = value % alignment;
	return rem ? (value - rem + alignment) : value;
}

// divides a by b, rounding the result upwards to an integer
inline ull integerCeilDivide(ull a, ull b)
{
	return a ? ((a - 1) / b + 1) : 0;
}