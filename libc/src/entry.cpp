#include <syscall.h>
#include <mem.h>

extern int main();

extern "C" void entry()
{
	// initialize heap
	Memory::selectedHeap = Memory::Heap::build((void *)0x7f0000000000, 0x10000);

	// call main
	exit(main());
}