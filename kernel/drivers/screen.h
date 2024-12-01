#pragma once
#include <screen.h>

namespace Screen
{
	void Initialize();
	void Cleanup();

	void driver_clear();
	void driver_print(const char *msg);
	void driver_print(char ch);

	void driver_paint(byte line, byte col, Cell::Color color);

	namespace Cursor
	{
		void driver_enable(byte start = 0xf, byte end = 0xd);
		void driver_disable();
	}
};