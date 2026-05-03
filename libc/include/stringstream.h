#pragma once

#include <iostream.h>
#include <string.h>

namespace std
{
	class stringstream : public ostream
	{
		string contents;
		ull allocationUnit;

		void write(const byte* buffer, ull len) override
		{
			// try to make space
			if (contents.getSize() + len > contents.getCapacity())
			{
				contents.reserve(contents.getSize() + allocationUnit *
					integerCeilDivide(contents.getSize() + len - contents.getCapacity(), allocationUnit)
				);
			}

			contents.append((const char*)buffer, len);
		}

	public:
		static constexpr ull defaultAllocationUnit = 128;

		inline stringstream(ull allocationUnit = defaultAllocationUnit) : allocationUnit(allocationUnit)
		{
			contents.reserve(allocationUnit);
		}

		inline void clear() { contents.erase(); }
		inline const string &getBuffer() { return contents; }
	};
}