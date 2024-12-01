#include "keyboard.h"
#include "../cpu/idt.h"
#include "../utils/isriostream.h"
#include "../core/sys.h"

using namespace ISR::std;

namespace Keyboard
{
	static constexpr int dataPort = 0x60,
						 statusPort = 0x64,
						 cmdPort = 0x64;

	bool expectingCommandResult = false,
		 sendDataNext;
	word lastCommand;
	KeyEvent *eventQueue;
	byte queueStart = 0, queueEnd = 0;

	void EventListener(registers_t &);

	void Initialize()
	{
		eventQueue = new KeyEvent[256];
		if (eventQueue == nullptr)
		{
			cout << "Failed to allocate keyboard buffer\n";
			System::blueScreen();
		}
		IDT::registerIrqHandler(1, EventListener);
	}
	void CleanUp()
	{
		delete[] eventQueue;
	}

	void waitForWrite()
	{
		byte status;
		do
		{
			status = inb(statusPort);
		} while (status & 0b10 != 0);
	}

	ModKeys currModKeys;
	void insertEvent(const KeyEvent &event) { eventQueue[queueEnd++] = event; }
	KeyCode scancodeToKeycodeMap[2][0x80]{
		{
			/*0x00*/ KeyCode::unknown, // '?',
			KeyCode::Escape,		   // 0x1b,
			KeyCode::alpha1,
			KeyCode::alpha2,
			KeyCode::alpha3,
			KeyCode::alpha4,
			KeyCode::alpha5,
			KeyCode::alpha6,
			KeyCode::alpha7,
			KeyCode::alpha8,
			KeyCode::alpha9,
			KeyCode::alpha0,
			KeyCode::alpha_minus,
			KeyCode::alpha_equal,
			KeyCode::backspace,
			KeyCode::tab,

			/*0x10*/ KeyCode::Q,
			KeyCode::W,
			KeyCode::E,
			KeyCode::R,
			KeyCode::T,
			KeyCode::Y,
			KeyCode::U,
			KeyCode::I,
			KeyCode::O,
			KeyCode::P,
			KeyCode::bracketOpen,
			KeyCode::bracketClosed,
			KeyCode::enter,
			KeyCode::leftControl, // '?',
			KeyCode::A,
			KeyCode::S,

			/*0x20*/ KeyCode::D,
			KeyCode::F,
			KeyCode::G,
			KeyCode::H,
			KeyCode::J,
			KeyCode::K,
			KeyCode::L,
			KeyCode::semicolon,
			KeyCode::singleQuote,
			KeyCode::unknown,	// '`',
			KeyCode::leftShift, // '?',
			KeyCode::backslash,
			KeyCode::Z,
			KeyCode::X,
			KeyCode::C,
			KeyCode::V,

			/*0x30*/ KeyCode::B,
			KeyCode::N,
			KeyCode::M,
			KeyCode::comma,
			KeyCode::alpha_dot,
			KeyCode::slash,
			KeyCode::rightShift, // '?',
			KeyCode::unknown,	 // '*',
			KeyCode::leftAlt,	 // '?',
			KeyCode::space,		 // ' ',
			KeyCode::capsLock,	 // '?',
			KeyCode::unknown,	 // '?',
			KeyCode::unknown,	 // '?',
			KeyCode::unknown,	 // '?',
			KeyCode::unknown,	 // '?',
			KeyCode::unknown,	 // '?',

			/*0x40*/ KeyCode::unknown, // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::numlock,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::numpad_7,
			KeyCode::numpad_8,
			KeyCode::numpad_9,
			KeyCode::numpad_minus,
			KeyCode::numpad_4,
			KeyCode::numpad_5,
			KeyCode::numpad_6,
			KeyCode::numpad_plus,
			KeyCode::numpad_1,

			/*0x50*/ KeyCode::numpad_2,
			KeyCode::numpad_3,
			KeyCode::numpad_0,
			KeyCode::numpad_dot,
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',
			KeyCode::unknown, // '?',

			/*0x60*/ KeyCode::unknown, // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',

			/*0x70*/ KeyCode::unknown, // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
			KeyCode::unknown,		   // '?',
		},
		{
			// extended keycodes

			/*0x00*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x10*/ KeyCode::previousTrack,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::nextTrack,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::numpad_enter,
			KeyCode::rightControl,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x20*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x30*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::numpad_slash,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::rightAlt,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x40*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::arrowUp,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::arrowLeft,
			KeyCode::unknown,
			KeyCode::arrowRight,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x50*/ KeyCode::arrowDown,
			KeyCode::unknown,
			KeyCode::insert,
			KeyCode::del,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::windows,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x60*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,

			/*0x70*/ KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
			KeyCode::unknown,
		}};

	void EventListener(registers_t &)
	{
		static bool extendedScancode = false;
		byte scanCode = inb(dataPort);

		if (expectingCommandResult)
		{
			switch ((Response)scanCode)
			{
			case Response::error1:
			case Response::error2:
			case Response::selfTestFailed:
				expectingCommandResult = false;
				break;
			case Response::selfTestPassed:
			case Response::echo:
				break;
			case Response::ack:
				if (sendDataNext)
					sendCommand();
				else
					expectingCommandResult = false;
				break;
			case Response::resend:
				if (sendDataNext)
					sendCommand((Command)(lastCommand & 0xff), lastCommand >> 8);
				else
					sendCommand();
				break;
			}
		}
		else
		{
			if (scanCode == 0xe0)
				extendedScancode = true;
			else
			{
				// cout << "Keyboard event: ";
				// if (extendedScancode)
				// 	cout << "0xE0 ";
				// if (scanCode >= 0x80)
				// 	cout << "0x80 | ";
				// cout << ostream::base::hex << "0x" << (uint)(0x7f & scanCode) << ostream::base::dec << "\n";

				if (!checkFullQueue())
				{
					byte releasedFlag = scanCode & 0x80;
					scanCode &= 0x7f;
					KeyCode key = scancodeToKeycodeMap[extendedScancode][scanCode];
					if (key == KeyCode::unknown && !releasedFlag)
					{
						cout << "Unknown keycode: 0x";
						if (extendedScancode)
							cout << "e0";
						cout << std::ostream::base::hex << scanCode << std::ostream::base::dec << '\n';
					}
					key = KeyCode((byte)key | releasedFlag);
					KeyEvent event = KeyEvent(key, currModKeys);
					insertEvent(event);

					switch (event.getKeyCode())
					{
					case KeyCode::leftShift:
						releasedFlag ? currModKeys.clearLeftShift() : currModKeys.setLeftShift();
						break;
					case KeyCode::leftAlt:
						releasedFlag ? currModKeys.clearLeftAlt() : currModKeys.setLeftAlt();
						break;
					case KeyCode::leftControl:
						releasedFlag ? currModKeys.clearLeftCtrl() : currModKeys.setLeftCtrl();
						break;
					case KeyCode::rightShift:
						releasedFlag ? currModKeys.clearRightShift() : currModKeys.setRightShift();
						break;
					case KeyCode::rightAlt:
						releasedFlag ? currModKeys.clearRightAlt() : currModKeys.setRightAlt();
						break;
					case KeyCode::rightControl:
						releasedFlag ? currModKeys.clearRightCtrl() : currModKeys.setRightCtrl();
						break;
					case KeyCode::capsLock:
						if (!releasedFlag)
						{
							if (currModKeys.getCapsLock())
								currModKeys.clearCapsLock();
							else
								currModKeys.setCapsLock();
							sendCommand(Command::setLED, (currModKeys.getCapsLock() << 2) | (currModKeys.getNumLock() << 1));
						}
						break;
					case KeyCode::numlock:
						if (!releasedFlag)
						{
							if (currModKeys.getNumLock())
								currModKeys.clearNumLock();
							else
								currModKeys.setNumLock();
							sendCommand(Command::setLED, (currModKeys.getCapsLock() << 2) | (currModKeys.getNumLock() << 1));
						}
						break;
					}
				}
				if (extendedScancode)
					extendedScancode = false;
			}
		}
	}

	inline bool checkCharQueue() { return queueStart != queueEnd; }
	inline bool checkFullQueue() { return byte(queueEnd + 1) == queueStart; }

	KeyEvent driver_getKeyEvent()
	{
		if (checkCharQueue())
			return eventQueue[queueStart++];
		return KeyEvent();
	}
	KeyEvent driver_getKeyPressedEvent()
	{
		while (checkCharQueue())
		{
			KeyEvent &event = eventQueue[queueStart++];
			if (event.isPressed())
				return event;
		}
		return KeyEvent();
	}
	KeyEvent driver_getKeyReleasedEvent()
	{
		while (checkCharQueue())
		{
			KeyEvent &event = eventQueue[queueStart++];
			if (event.isReleased())
				return event;
		}
		return KeyEvent();
	}

	void sendCommand()
	{
		waitForWrite();
		sendDataNext = false;
		outb(dataPort, lastCommand >> 8);
	}
	void sendCommand(Command command, byte data)
	{
		lastCommand = (byte)command | (data << 8);
		expectingCommandResult = true;

		switch (command)
		{
		case Command::setLED:
			waitForWrite();
			sendDataNext = true;
			outb(dataPort, (byte)command);
			break;
		default:
			expectingCommandResult = false;
			break;
		}
	}
}