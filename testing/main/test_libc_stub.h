#include <iostream.h>
#include "../libc/src/globals/mem.cpp"

extern "C" void __cxa_pure_virtual()
{
	// crash intentionally
	*(byte*)nullptr = 0;
}
namespace std
{
    ostream cout;
}