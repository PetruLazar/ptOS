#include "sys.h"
#include <iostream.h>
#include "../cpu/interrupt/idt.h"
#include "../drivers/keyboard.h"

using namespace std;

namespace System
{
	void pause(bool echo)
	{
		if (echo)
			cout << "Press any key to continue...";

		registers_t regs;
		regs.rdi = false; // use non-blocking calls, as this will be used in kernel code, with shcheduler disabled
		Keyboard::KeyEvent *event = (Keyboard::KeyEvent *)&regs.rax;
		while (true)
		{
			Keyboard::driver_getKeyPressedEvent(regs, false);

			if (event->getKeyCode() != Keyboard::KeyCode::unknown)
				break;
		}

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