#pragma once
#include "../../drivers/screen.h"
#include "../../drivers/keyboard.h"

class Snake
{
	static void spawnApple();

	static void Build();
	static void Redraw();
	static void Cycle();

public:
	static void Play();
};