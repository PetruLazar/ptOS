#pragma once
#include "../cpu/ports.h"

class Keyboard
{
public:
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

	class KeyEvent
	{
	public:
		enum class KeyCode : byte
		{
			f1,
			f2,
			f3,
			f4,
			f5,
			f6,
			f7,
			f8,
			f9,
			f10,
			f11,
			f12,

			Escape,
			printScreen,
			insert,
			del,
			capsLock,
			tab,

			alpha_minus,
			alpha_equal,
			backspace,
			bracketOpen,
			bracketClosed,
			backslash,
			semicolon,
			singleQuote,
			enter,
			comma,
			alpha_dot,
			slash,

			alpha0,
			alpha1,
			alpha2,
			alpha3,
			alpha4,
			alpha5,
			alpha6,
			alpha7,
			alpha8,
			alpha9,
			numpad_0,
			numpad_1,
			numpad_2,
			numpad_3,
			numpad_4,
			numpad_5,
			numpad_6,
			numpad_7,
			numpad_8,
			numpad_9,

			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,

			leftShift,
			rightShift,
			leftControl,
			Fn,
			windows,
			leftAlt,
			space,
			rightAlt,
			rightControl,
			arrowUp,
			arrowDown,
			arrowLeft,
			arrowRight,
			numlock,

			numpad_minus,
			numpad_plus,
			numpad_star,
			numpad_slash,
			numpad_enter,
			numpad_dot,

			previousTrack,
			nextTrack,

			unknown = 0x7f,
			keyReleasedFlag = 0x80,
		} keyCode;

		class ModKeys
		{
		public:
			enum MaskBits : byte
			{
				leftShift = 0x1,
				leftAlt = 0x2,
				leftCtrl = 0x4,
				rightShift = 0x8,
				rightAlt = 0x10,
				rightCtrl = 0x20,
				capsLock = 0x40,
				numLock = 0x80
			} mask;

			inline ModKeys() : mask(MaskBits(0)) {}

			inline bool getLeftShift() { return mask & leftShift; }
			inline bool getRightShift() { return mask & rightShift; }
			inline bool getShift() { return mask & (leftShift | rightShift); }

			inline bool getLeftAlt() { return mask & leftAlt; }
			inline bool getRightAlt() { return mask & rightAlt; }
			inline bool getAlt() { return mask & (leftAlt | rightAlt); }

			inline bool getLeftCtrl() { return mask & leftCtrl; }
			inline bool getRightCtrl() { return mask & rightCtrl; }
			inline bool getCtrl() { return mask & (leftCtrl | rightCtrl); }

			inline bool getCapsLock() { return mask & capsLock; }
			inline bool getNumLock() { return mask & numLock; }

			inline void setLeftShift() { mask = MaskBits(mask | leftShift); }
			inline void clearLeftShift() { mask = MaskBits(mask & ~leftShift); }
			inline void setRightShift() { mask = MaskBits(mask | rightShift); }
			inline void clearRightShift() { mask = MaskBits(mask & ~rightShift); }
			inline void setLeftAlt() { mask = MaskBits(mask | leftAlt); }
			inline void clearLeftAlt() { mask = MaskBits(mask & ~leftAlt); }
			inline void setRightAlt() { mask = MaskBits(mask | rightAlt); }
			inline void clearRightAlt() { mask = MaskBits(mask & ~rightAlt); }
			inline void setLeftCtrl() { mask = MaskBits(mask | leftCtrl); }
			inline void clearLeftCtrl() { mask = MaskBits(mask & ~leftCtrl); }
			inline void setRightCtrl() { mask = MaskBits(mask | rightCtrl); }
			inline void clearRightCtrl() { mask = MaskBits(mask & ~rightCtrl); }

			inline void setCapsLock() { mask = MaskBits(mask | capsLock); }
			inline void clearCapsLock() { mask = MaskBits(mask & ~capsLock); }
			inline void setNumLock() { mask = MaskBits(mask | numLock); }
			inline void clearNumLock() { mask = MaskBits(mask & ~numLock); }
		} modKeys;

		inline KeyEvent() : keyCode(KeyCode::unknown), modKeys() {}
		inline KeyEvent(KeyCode keyCode, ModKeys modKeys) : keyCode(keyCode), modKeys(modKeys) {}

		inline KeyCode getKeyCode() { return KeyCode((byte)keyCode & ~(byte)KeyCode::keyReleasedFlag); }
		inline bool isReleased() { return (byte)keyCode & (byte)KeyCode::keyReleasedFlag; }
		inline bool isPressed() { return !isReleased(); }
		char getChar();
	};

	static void Initialize();
	static void EventListener();
	static void sendCommand(Command command, byte data);

	static bool checkCharQueue();
	static bool checkFullQueue();
	static KeyEvent getKeyEvent();
	static KeyEvent getKeyPressedEvent();
	static KeyEvent getKeyReleasedEvent();
};