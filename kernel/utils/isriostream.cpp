#include "isriostream.h"

class isr_ostream : public std::ostream
{
	void write(const byte* buffer, ull len) override
	{
		Screen::driver_print((const char*)buffer, len);
	}
} _isrcout;

namespace std
{
	ostream& isrcout = _isrcout;
}