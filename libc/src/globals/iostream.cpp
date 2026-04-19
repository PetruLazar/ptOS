#include <iostream.h>

class console_ostream : public std::ostream
{
	void write(const byte* buffer, ull len) override
	{
		Screen::print((const char*)buffer, len);
	}
} _cout;
class null_ostream : public std::ostream
{
	void write(const byte* buffer, ull len) override
	{
		// do nothing
	}
} _nullout;


namespace std
{
	istream cin;
	ostream& cout = _cout;
	ostream& nullout = _nullout;
}