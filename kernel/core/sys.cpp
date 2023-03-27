#include "sys.h"
#include <iostream.h>
#include "../drivers/keyboard.h"
#include "../cpu/idt.h"

using namespace std;

namespace System
{
	void pause(bool echo)
	{
		if (echo)
			cout << "Press any key to continue...";
		while (Keyboard::getKeyPressedEvent().getKeyCode() == Keyboard::KeyCode::unknown)
			;
		if (echo)
			cout << '\n';
	}
	void blueScreen()
	{
		disableInterrupts();
		while (true)
			;
	}
}