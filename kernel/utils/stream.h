#pragma once
#include "string.h"

namespace std
{
	template <class T>
	class basic_stream
	{
		vector<string *> contents;

	public:
		inline basic_stream() : contents(10) {}

		inline void insert(const T *buffer, ull len);
		inline void extract(T *buffer, ull len);
		inline void skip(ull count);
		inline void skipUntil();
	};
}