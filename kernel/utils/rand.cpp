#include "rand.h"

int current = 0;

void Random::setSeed(int seed) { current = seed; }
int Random::get()
{
	current = current * 0x216B9CE9 + 0x69B56F01;
	return current;
}