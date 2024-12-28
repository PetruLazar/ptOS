#pragma once
#include <keyboard.h>
#include "../cpu/ports.h"
#include "../cpu/idt.h"

namespace Keyboard
{
	enum class Response : byte
	{
		error1 = 0x00,
		selfTestPassed = 0xaa,
		echo = 0xee,
		ack = 0xfa,
		selfTestFailed = 0xfc, // and 0xfd
		resend = 0xfe,
		error2 = 0xff
	};
	enum class Command : byte
	{
		setLED = 0xed,
		echo = 0xee,
		scancodeSet = 0xf0,
		identify = 0xf2,
		setTypematicRateAndDelay = 0xf3,
		enableScanning = 0xf4,
		disableScanning = 0xf5,
		setDefaultParameters = 0xf6,
		setAllKeysToTypematicAutorepeat = 0xf7,			   // keycode set 3 specific
		setAllKeysToMakeRelease = 0xf8,					   // keycode set 3 specific
		setAllKeysToMakeOnly = 0xf9,					   // keycode set 3 specific
		setAllKeysToTypematicAutorepeatMakeRelease = 0xfa, // keycode set 3 specific
		setSpecificKeyToTypematicAutorepeat = 0xfb,		   // keycode set 3 specific
		setSpecificKeyToMakeRelease = 0xfc,				   // keycode set 3 specific
		setSpecificKeyToMakeOnly = 0xfd,				   // keycode set 3 specific
		resendLastByte = 0xfe,
		resetAndSelfTest = 0xff
	};

	void Initialize();
	void CleanUp();
	// void EventListener(registers_t &regs);
	void sendCommand();
	void sendCommand(Command command, byte data);

	bool checkCharQueue();
	bool checkFullQueue();

	void driver_getKeyEvent(registers_t &regs, bool expectingCharOnly);
	void driver_getKeyPressedEvent(registers_t &regs, bool expectingCharOnly);
	void driver_getKeyReleasedEvent(registers_t &regs, bool expectingCharOnly);

	void Syscall(registers_t &);
};