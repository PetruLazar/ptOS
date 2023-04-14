#include <syscall.h>

extern int main();

extern "C" void entry()
{
	// initialize heap

	exit(main());
}