#include "keyboard.h"
#include "screen.h"
#include "../cpu/idt.h"
#include "../utils/string.h"
#include "../utils/iostream.h"
#include "../core/sys.h"

using namespace std;

static constexpr int dataPort = 0x60,
					 statusPort = 0x64,
					 cmdPort = 0x64;

bool expectingCommandResult = false;
Keyboard::KeyEvent *eventQueue;
byte queueStart = 0, queueEnd = 0;

void Keyboard::Initialize()
{
	eventQueue = new KeyEvent[256];
	if (eventQueue == nullptr)
	{
		cout << "Failed to allocate keyboard buffer\n";
		System::blueScreen();
	}
	IDT::registerIrqHandler(1, EventListener);
}

void waitForWrite()
{
	byte status;
	do
	{
		status = inb(statusPort);
	} while (status & 0b10 != 0);
}

// make a keyToCharMap
char Keyboard::KeyEvent::getChar()
{
	switch (getKeyCode())
	{
	case KeyCode::A:
	case KeyCode::B:
	case KeyCode::C:
	case KeyCode::D:
	case KeyCode::E:
	case KeyCode::F:
	case KeyCode::G:
	case KeyCode::H:
	case KeyCode::I:
	case KeyCode::J:
	case KeyCode::K:
	case KeyCode::L:
	case KeyCode::M:
	case KeyCode::N:
	case KeyCode::O:
	case KeyCode::P:
	case KeyCode::Q:
	case KeyCode::R:
	case KeyCode::S:
	case KeyCode::T:
	case KeyCode::U:
	case KeyCode::V:
	case KeyCode::W:
	case KeyCode::X:
	case KeyCode::Y:
	case KeyCode::Z:
		return (char)keyCode - (char)KeyCode::A + (modKeys.getShift() ^ modKeys.getCapsLock() ? 'A' : 'a');
	case KeyCode::numpad_0:
	case KeyCode::numpad_1:
	case KeyCode::numpad_2:
	case KeyCode::numpad_3:
	case KeyCode::numpad_4:
	case KeyCode::numpad_5:
	case KeyCode::numpad_6:
	case KeyCode::numpad_7:
	case KeyCode::numpad_8:
	case KeyCode::numpad_9:
		if (modKeys.getNumLock())
			return '0' + ((char)keyCode - (char)KeyCode ::numpad_0);
		return '?';
	case KeyCode::alpha0:
	case KeyCode::alpha1:
	case KeyCode::alpha2:
	case KeyCode::alpha3:
	case KeyCode::alpha4:
	case KeyCode::alpha5:
	case KeyCode::alpha6:
	case KeyCode::alpha7:
	case KeyCode::alpha8:
	case KeyCode::alpha9:
	{
		char d = (char)keyCode - (char)KeyCode::alpha0;
		if (modKeys.getShift())
			return ")!@#$%^&*("[d];
		return '0' + d;
	}
	case KeyCode::numpad_minus:
	case KeyCode::numpad_plus:
	case KeyCode::numpad_star:
	case KeyCode::numpad_slash:
	case KeyCode::numpad_enter:
	case KeyCode::numpad_dot:
	{
		char d = (char)keyCode - (char)KeyCode::numpad_minus;
		return "-+*/\r."[d];
	}
	case KeyCode::alpha_minus:
	case KeyCode::alpha_equal:
	case KeyCode::backspace:
	case KeyCode::bracketOpen:
	case KeyCode::bracketClosed:
	case KeyCode::backslash:
	case KeyCode::semicolon:
	case KeyCode::singleQuote:
	case KeyCode::enter:
	case KeyCode::comma:
	case KeyCode::alpha_dot:
	case KeyCode::slash:
	{
		char d = (char)keyCode - (char)KeyCode::alpha_minus;
		if (modKeys.getShift())
			return "_+\b{}|:\"\r<>?"[d];
		return "-=\b[]\\;'\r,./"[d];
	}
	case KeyCode::space:
		return ' ';
	}
	return 0;
}

Keyboard::KeyEvent::ModKeys currModKeys;
void insertEvent(const Keyboard::KeyEvent &event)
{
	eventQueue[queueEnd++] = event;
	// cout << "Key inserted, queueEnd becomes " << queueEnd << '\n';
}
Keyboard::KeyEvent::KeyCode scancodeToKeycodeMap[2][0x80]{
	{
		/*0x00*/ Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::Escape,		   // 0x1b,
		Keyboard::KeyEvent::KeyCode::alpha1,
		Keyboard::KeyEvent::KeyCode::alpha2,
		Keyboard::KeyEvent::KeyCode::alpha3,
		Keyboard::KeyEvent::KeyCode::alpha4,
		Keyboard::KeyEvent::KeyCode::alpha5,
		Keyboard::KeyEvent::KeyCode::alpha6,
		Keyboard::KeyEvent::KeyCode::alpha7,
		Keyboard::KeyEvent::KeyCode::alpha8,
		Keyboard::KeyEvent::KeyCode::alpha9,
		Keyboard::KeyEvent::KeyCode::alpha0,
		Keyboard::KeyEvent::KeyCode::alpha_minus,
		Keyboard::KeyEvent::KeyCode::alpha_equal,
		Keyboard::KeyEvent::KeyCode::backspace,
		Keyboard::KeyEvent::KeyCode::tab,

		/*0x10*/ Keyboard::KeyEvent::KeyCode::Q,
		Keyboard::KeyEvent::KeyCode::W,
		Keyboard::KeyEvent::KeyCode::E,
		Keyboard::KeyEvent::KeyCode::R,
		Keyboard::KeyEvent::KeyCode::T,
		Keyboard::KeyEvent::KeyCode::Y,
		Keyboard::KeyEvent::KeyCode::U,
		Keyboard::KeyEvent::KeyCode::I,
		Keyboard::KeyEvent::KeyCode::O,
		Keyboard::KeyEvent::KeyCode::P,
		Keyboard::KeyEvent::KeyCode::bracketOpen,
		Keyboard::KeyEvent::KeyCode::bracketClosed,
		Keyboard::KeyEvent::KeyCode::enter,
		Keyboard::KeyEvent::KeyCode::leftControl, // '?',
		Keyboard::KeyEvent::KeyCode::A,
		Keyboard::KeyEvent::KeyCode::S,

		/*0x20*/ Keyboard::KeyEvent::KeyCode::D,
		Keyboard::KeyEvent::KeyCode::F,
		Keyboard::KeyEvent::KeyCode::G,
		Keyboard::KeyEvent::KeyCode::H,
		Keyboard::KeyEvent::KeyCode::J,
		Keyboard::KeyEvent::KeyCode::K,
		Keyboard::KeyEvent::KeyCode::L,
		Keyboard::KeyEvent::KeyCode::semicolon,
		Keyboard::KeyEvent::KeyCode::singleQuote,
		Keyboard::KeyEvent::KeyCode::unknown,	// '`',
		Keyboard::KeyEvent::KeyCode::leftShift, // '?',
		Keyboard::KeyEvent::KeyCode::backslash,
		Keyboard::KeyEvent::KeyCode::Z,
		Keyboard::KeyEvent::KeyCode::X,
		Keyboard::KeyEvent::KeyCode::C,
		Keyboard::KeyEvent::KeyCode::V,

		/*0x30*/ Keyboard::KeyEvent::KeyCode::B,
		Keyboard::KeyEvent::KeyCode::N,
		Keyboard::KeyEvent::KeyCode::M,
		Keyboard::KeyEvent::KeyCode::comma,
		Keyboard::KeyEvent::KeyCode::alpha_dot,
		Keyboard::KeyEvent::KeyCode::slash,
		Keyboard::KeyEvent::KeyCode::rightControl, // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '*',
		Keyboard::KeyEvent::KeyCode::leftAlt,	   // '?',
		Keyboard::KeyEvent::KeyCode::space,		   // ' ',
		Keyboard::KeyEvent::KeyCode::capsLock,	   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,	   // '?',

		/*0x40*/ Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::numpad_7,
		Keyboard::KeyEvent::KeyCode::numpad_8,
		Keyboard::KeyEvent::KeyCode::numpad_9,
		Keyboard::KeyEvent::KeyCode::numpad_minus,
		Keyboard::KeyEvent::KeyCode::numpad_4,
		Keyboard::KeyEvent::KeyCode::numpad_5,
		Keyboard::KeyEvent::KeyCode::numpad_6,
		Keyboard::KeyEvent::KeyCode::numpad_plus,
		Keyboard::KeyEvent::KeyCode::numpad_1,

		/*0x50*/ Keyboard::KeyEvent::KeyCode::numpad_2,
		Keyboard::KeyEvent::KeyCode::numpad_3,
		Keyboard::KeyEvent::KeyCode::numpad_0,
		Keyboard::KeyEvent::KeyCode::numpad_dot,
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown, // '?',

		/*0x60*/ Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',

		/*0x70*/ Keyboard::KeyEvent::KeyCode::unknown, // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
		Keyboard::KeyEvent::KeyCode::unknown,		   // '?',
	},
	{
		// extended keycodes

		/*0x00*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x10*/ Keyboard::KeyEvent::KeyCode::previousTrack,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::nextTrack,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::numpad_enter,
		Keyboard::KeyEvent::KeyCode::rightControl,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x20*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x30*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::numpad_slash,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::rightAlt,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x40*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::arrowUp,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::arrowLeft,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::arrowRight,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x50*/ Keyboard::KeyEvent::KeyCode::arrowDown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::insert,
		Keyboard::KeyEvent::KeyCode::del,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::windows,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x60*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,

		/*0x70*/ Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
		Keyboard::KeyEvent::KeyCode::unknown,
	}

};

void Keyboard::EventListener()
{
	static bool extendedScancode = false;
	byte scanCode = inb(dataPort);

	if (expectingCommandResult)
	{
		switch ((Response)scanCode)
		{
		case Response::error1:
		case Response::error2:
		case Response::selfTestPassed:
		case Response::selfTestFailed:
		case Response::echo:
		case Response::ack:
		case Response::resend:
			expectingCommandResult = false;
			break;
		}
	}
	else
	{
		if (scanCode == 0xe0)
			extendedScancode = true;
		else
		{
			/*cout << "Keyboard event: ";
			if (extendedScancode)
				cout << "0xE0 ";
			if (scanCode >= 0x80)
				cout << "0x80 | ";
			cout << ostream::base::hex << "0x" << (uint)(0x7f & scanCode) << ostream::base::dec << "\n";*/

			if (!checkFullQueue())
			{
				byte releasedFlag = scanCode & 0x80;
				scanCode &= 0x7f;
				KeyEvent::KeyCode key = scancodeToKeycodeMap[extendedScancode][scanCode];
				key = KeyEvent::KeyCode((byte)key | releasedFlag);
				KeyEvent event = KeyEvent(key, currModKeys);
				insertEvent(event);

				// cout << "DBG: " << extendedScancode << ' ' << scanCode << '\n';
				// cout << "Key event " << (uint)event.getKeyCode() << '\n';
				switch (event.getKeyCode())
				{
				case KeyEvent::KeyCode::leftShift:
					releasedFlag ? currModKeys.clearLeftShift() : currModKeys.setLeftShift();
					break;
				case KeyEvent::KeyCode::leftAlt:
					releasedFlag ? currModKeys.clearLeftAlt() : currModKeys.setLeftAlt();
					break;
				case KeyEvent::KeyCode::leftControl:
					releasedFlag ? currModKeys.clearLeftCtrl() : currModKeys.setLeftCtrl();
					break;
				case KeyEvent::KeyCode::rightShift:
					releasedFlag ? currModKeys.clearRightShift() : currModKeys.setRightShift();
					break;
				case KeyEvent::KeyCode::rightAlt:
					releasedFlag ? currModKeys.clearRightAlt() : currModKeys.setRightAlt();
					break;
				case KeyEvent::KeyCode::rightControl:
					releasedFlag ? currModKeys.clearRightCtrl() : currModKeys.setRightCtrl();
					break;
				case KeyEvent::KeyCode::capsLock:
					if (!releasedFlag)
					{
						// to-do: turn the LED on or off
						if (currModKeys.getCapsLock())
							currModKeys.clearCapsLock();
						else
							currModKeys.setCapsLock();
					}
					break;
				case KeyEvent::KeyCode::numlock:
					if (!releasedFlag)
					{
						// to-do: turn the LED on or off
						if (currModKeys.getNumLock())
							currModKeys.clearNumLock();
						else
							currModKeys.setNumLock();
					}
					break;
				}
			}
			if (extendedScancode)
				extendedScancode = false;
		}
	}
}

inline bool Keyboard::checkCharQueue() { return queueStart != queueEnd; }
inline bool Keyboard::checkFullQueue()
{
	byte tmp = queueEnd + 1;
	return tmp == queueStart;
}
Keyboard::KeyEvent Keyboard::getKeyEvent()
{
	if (checkCharQueue())
		return eventQueue[queueStart++];
	return KeyEvent();
}
Keyboard::KeyEvent Keyboard::getKeyPressedEvent()
{
	if (checkCharQueue())
	{
		KeyEvent &event = eventQueue[queueStart++];
		if (event.isPressed())
			return event;
	}
	return KeyEvent();
}
Keyboard::KeyEvent Keyboard::getKeyReleasedEvent()
{
	if (checkCharQueue())
	{
		KeyEvent &event = eventQueue[queueStart++];
		if (event.isReleased())
			return event;
	}
	return KeyEvent();
}

/*void Keyboard::sendCommand(Command command, byte data)
{
	if (command == Command::identify)
	{
		waitForWrite();
		outb(dataPort, (byte)command);
		return;
	}

	switch (command)
	{
	case Command::identify:
		waitForWrite();
		outb(cmdPort, (byte)command);
		break;
	case Command::setLED:
		waitForWrite();
		outb(cmdPort, (byte)command);
		waitForWrite();
		outb(dataPort, 0b010);
		break;
	case Command::scancodeSet:
		waitForWrite();
		outb(dataPort, (byte)command);
		waitForWrite();
		outb(dataPort, data);
		break;

	default:
		break;
	}
}*/