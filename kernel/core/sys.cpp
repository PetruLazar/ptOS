#include "sys.h"
#include <iostream.h>
#include "../cpu/idt.h"
#include "../drivers/keyboard.h"

using namespace std;

namespace System
{
	void pause(bool echo)
	{
		if (echo)
			cout << "Press any key to continue...";
		while (Keyboard::driver_getKeyPressedEvent().getKeyCode() == Keyboard::KeyCode::unknown)
			;
		if (echo)
			cout << '\n';
	}
	void blueScreen()
	{
		disableInterrupts();
		Screen::paint(Vector2b(0, 0), Vector2b(Screen::screenWidth, Screen::screenHeight), Screen::Cell::Color::blue);

		while (true)
			;
	}
}