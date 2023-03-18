#include "rand.h"

unsigned int current = 0;

void Random::setSeed(unsigned int seed) { current = seed; }
unsigned int Random::get()
{
	current = current * 0x216B9CE9 + 0x69B56F01;
	return current;
}