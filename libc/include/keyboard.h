#pragma once
#include <types.h>

namespace Keyboard
{
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
	};

	struct ModKeys
	{
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
	};

	struct KeyEvent
	{
		KeyCode keyCode;
		ModKeys modKeys;

		inline KeyEvent() : keyCode(KeyCode::unknown), modKeys() {}
		inline KeyEvent(KeyCode keyCode, ModKeys modKeys) : keyCode(keyCode), modKeys(modKeys) {}

		inline KeyCode getKeyCode() { return KeyCode((byte)keyCode & ~(byte)KeyCode::keyReleasedFlag); }
		inline bool isReleased() { return (byte)keyCode & (byte)KeyCode::keyReleasedFlag; }
		inline bool isPressed() { return !isReleased(); }

		char getChar()
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
				return 0;
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
	};
}

#include <syscall.h>