#include <rand.h>

namespace Random
{
	unsigned int current = 0;

	void setSeed(unsigned int seed) { current = seed; }
	unsigned int get()
	{
		return current = current * 0x216B9CE9 + 0x69B56F01;
	}
}