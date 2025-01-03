#include "keyboard.h"
#include "../cpu/idt.h"
#include "../utils/isriostream.h"
#include "../core/sys.h"
#include "../core/scheduler.h"

using namespace ISR::std;

namespace Keyboard
{
	static constexpr int dataPort = 0x60,
						 statusPort = 0x64,
						 cmdPort = 0x64;

	class KeyboardDriverRequest
	{
	public:
		Thread *blockedThread;
		enum EventType : byte
		{
			any,
			pressed,
			released
		} eventRequested;
		enum ValueType : byte
		{
			event,
			character
		} valueRequested;

		KeyboardDriverRequest(Thread *blockedThread, EventType eventRequested, ValueType valueRequested)
			: blockedThread(blockedThread), eventRequested(eventRequested), valueRequested(valueRequested) {}
		KeyboardDriverRequest() {}
	};

	bool expectingCommandResult = false,
		 sendDataNext;
	byte queueStart = 0, queueEnd = 0;
	word lastCommand;
	KeyEvent *eventQueue;
	Thread *keyboardDriverThread;
	std::vector<KeyboardDriverRequest> *requests;

	void EventListener(registers_t &);

	void Initialize()
	{
		// initialize event queue
		eventQueue = new KeyEvent[256];
		if (eventQueue == nullptr)
		{
			cout << "Failed to allocate keyboard buffer\n";
			System::blueScreen();
		}
		IDT::registerIrqHandler(1, EventListener);

		// initialize keyboard driver thread
		// for now, use main kernel thread as keyboard thread
		keyboardDriverThread = Scheduler::getCurrentThread();

		// initialize request queue for threads that will be blocking on this driver
		requests = new std::vector<KeyboardDriverRequest>(4);
	}
	void CleanUp()
	{
		delete[] eventQueue;
		delete requests;
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
	void insertEvent(registers_t &regs, const KeyEvent &event)
	{
		eventQueue[queueEnd++] = event;

		// check if any thread waiting for an event needs to be unblocked
		if (requests->getSize() == 0)
			return; // no threads waiting for a KeyEvent

		KeyboardDriverRequest &req = requests->at(0);
		KeyEvent *returnValue = (KeyEvent *)&req.blockedThread->getRegs().rax;
		*returnValue = eventQueue[queueStart++]; // threads waiting, immediately dequeue event
		if (req.eventRequested == KeyboardDriverRequest::any ||
			(req.eventRequested == KeyboardDriverRequest::pressed) == returnValue->isPressed())
		{
			// unblock thread
			// update return value if needed
			if (req.valueRequested == KeyboardDriverRequest::character)
				req.blockedThread->getRegs().rax = returnValue->getChar();

			// call scheduler
			Scheduler::unblockThread(regs, keyboardDriverThread, req.blockedThread);

			// dequeue request
			requests->erase(0);
		}
	}
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
			// extended scancodes

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

	void EventListener(registers_t &regs)
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
					insertEvent(regs, event);

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

	void driver_getKeyEvent(registers_t &regs, bool expectingCharOnly)
	{
		KeyEvent *returnValue = (KeyEvent *)&regs.rax;
		bool blocking = regs.rdi;
		if (checkCharQueue())
		{
			// key event queue not empty, return immediately
			*returnValue = eventQueue[queueStart++];
			if (expectingCharOnly)
				regs.rax = returnValue->getChar();
			return;
		}

		// key event queue is empty
		if (blocking)
		{
			// block on the keyboard driver thread
			requests->push_back(KeyboardDriverRequest(Scheduler::getCurrentThread(), KeyboardDriverRequest::any, expectingCharOnly ? KeyboardDriverRequest::character : KeyboardDriverRequest::event));
			Scheduler::waitForThread(regs, keyboardDriverThread);
		}
		else
		{
			// return default value, do NOT block
			*returnValue = KeyEvent();
			if (expectingCharOnly)
				regs.rax = returnValue->getChar();
		}
	}
	void driver_getKeyPressedEvent(registers_t &regs, bool expectingCharOnly)
	{
		KeyEvent *returnValue = (KeyEvent *)&regs.rax;
		bool blocking = regs.rdi;
		while (checkCharQueue())
		{
			KeyEvent &event = eventQueue[queueStart++];
			if (event.isPressed()) // skip any event that is not a KeyPressedEvent
			{
				// key queue contains a KeyPressedEvent, return immediately
				*returnValue = event;
				if (expectingCharOnly)
					regs.rax = returnValue->getChar();
				return;
			}
		}

		// key event queue does not contain the desired event
		if (blocking)
		{
			// block on the keyboard driver thread
			requests->push_back(KeyboardDriverRequest(Scheduler::getCurrentThread(), KeyboardDriverRequest::pressed, expectingCharOnly ? KeyboardDriverRequest::character : KeyboardDriverRequest::event));
			Scheduler::waitForThread(regs, keyboardDriverThread);
		}
		else
		{
			// return default value, do NOT block
			*returnValue = KeyEvent();
			if (expectingCharOnly)
				regs.rax = returnValue->getChar();
		}
	}
	void driver_getKeyReleasedEvent(registers_t &regs, bool expectingCharOnly)
	{
		KeyEvent *returnValue = (KeyEvent *)&regs.rax;
		bool blocking = regs.rdi;
		while (checkCharQueue())
		{
			KeyEvent &event = eventQueue[queueStart++];
			if (event.isReleased()) // skip any event that is not a KeyReleasedEvent
			{
				// key queue contains a KeyReleasedEvent, return immediately
				*returnValue = event;
				if (expectingCharOnly)
					regs.rax = returnValue->getChar();
				return;
			}
		}

		// key event queue does not contain the desired event
		if (blocking)
		{
			// block on the keyboard driver thread
			requests->push_back(KeyboardDriverRequest(Scheduler::getCurrentThread(), KeyboardDriverRequest::released, expectingCharOnly ? KeyboardDriverRequest::character : KeyboardDriverRequest::event));
			Scheduler::waitForThread(regs, keyboardDriverThread);
		}
		else
		{
			// return default value, do NOT block
			*returnValue = KeyEvent();
			if (expectingCharOnly)
				regs.rax = returnValue->getChar();
		}
	}

	void Syscall(registers_t &regs)
	{
		switch (regs.rbx)
		{
		case SYSCALL_KEYBOARD_KEYEVENT:
		case SYSCALL_KEYBOARD_KEYEVENT_CHAR:
		{
			return driver_getKeyEvent(regs, regs.rbx == SYSCALL_KEYBOARD_KEYEVENT_CHAR);
		}
		break;
		case SYSCALL_KEYBOARD_KEYPRESSEDEVENT:
		case SYSCALL_KEYBOARD_KEYPRESSEDEVENT_CHAR:
		{
			return driver_getKeyPressedEvent(regs, regs.rbx == SYSCALL_KEYBOARD_KEYPRESSEDEVENT_CHAR);
		}
		break;
		case SYSCALL_KEYBOARD_KEYRELEASEDEVENT:
		case SYSCALL_KEYBOARD_KEYRELEASEDEVENT_CHAR:
		{
			return driver_getKeyReleasedEvent(regs, regs.rbx == SYSCALL_KEYBOARD_KEYRELEASEDEVENT_CHAR);
		}
		break;
		}
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