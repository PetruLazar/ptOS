#pragma once

namespace std
{
	template <class T>
	class list
	{
		class listEntry
		{
		public:
			listEntry *next;
			T obj;

			inline listEntry(listEntry *next, T obj) : next(next), obj(obj) {}
		} * first;

	public:
		inline list() : first(nullptr) {}

		inline void add(T val) {}
		inline void remove(T val);
	};
}