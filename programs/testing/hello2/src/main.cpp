#include <syscall.h>
#include <cstring.h>

extern "C" int main()
{
	for (int i = 0; i < 80; i++)
	{
		Screen::print('2');
		Time::sleep(100);
	}
	return 0;
}