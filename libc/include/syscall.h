#pragma once
#include <types.h>
#include <screen.h>
#include <keyboard.h>
#include <mem.h>

#define SYSCALL_BREAKPOINT 0
#define SYSCALL_SCREEN 1
#define SYSCALL_KEYBOARD 2
#define SYSCALL_FILESYSTEM 3
#define SYSCALL_PROGENV 4
#define SYSCALL_CURSOR 5
#define SYSCALL_TIME 6

#define SYSCALL_SCREEN_CLEAR 0
#define SYSCALL_SCREEN_PRINTSTR 1
#define SYSCALL_SCREEN_PRINTCH 2
#define SYSCALL_SCREEN_PAINT 3

#define SYSCALL_KEYBOARD_KEYEVENT 0
#define SYSCALL_KEYBOARD_KEYPRESSEDEVENT 1
#define SYSCALL_KEYBOARD_KEYRELEASEDEVENT 2
#define SYSCALL_KEYBOARD_KEYEVENT_CHAR 3
#define SYSCALL_KEYBOARD_KEYPRESSEDEVENT_CHAR 4
#define SYSCALL_KEYBOARD_KEYRELEASEDEVENT_CHAR 5

#define SYSCALL_FILESYSTEM_READFILE 0
#define SYSCALL_FILESYSTEM_WRITEFILE 1
// #define SYSCALL_FILESYSTEM_GET

#define SYSCALL_PROGENV_EXIT 0
#define SYSCALL_PROGENV_WAITFORTASK 1
#define SYSCALL_PROGENV_WAITFORTHREAD 2
// #define SYSCALL_PROGENV_ALLOCHEAP 3
// #define SYSCALL_PROGENV_HEAPFULL 4
// #define SYSCALL_PROGENV_HEAPCORRUPTION 5

#define SYSCALL_CURSOR_ENABLE 0
#define SYSCALL_CURSOR_DISABLE 1

#define SYSCALL_TIME_GET 0
#define SYSCALL_TIME_SLEEP 1

inline void syscall_breakpoint()
{
	asm("int $0x30"
		:
		: "a"(0));
}

namespace Screen
{
	inline void clear()
	{
		asm("int $0x30"
			:
			: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_CLEAR));
	}
	inline void print(char ch)
	{
		asm("int $0x30"
			:
			: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_PRINTCH), "D"(ch));
	}
	inline void print(const char *msg)
	{
		asm("int $0x30"
			:
			: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_PRINTSTR), "D"(msg));
	}
	inline void paint(byte line, byte col, Cell::Color color)
	{
		asm("int $0x30"
			:
			: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_PAINT), "D"(line), "S"(col), "d"(color));
	}

	namespace Cursor
	{
		inline void enable(byte start = 0xf, byte end = 0xd)
		{
			asm("int $0x30"
				:
				: "a"(SYSCALL_CURSOR), "b"(SYSCALL_CURSOR_ENABLE), "D"(start), "S"(end));
		}
		inline void disable()
		{
			asm("int $0x30"
				:
				: "a"(SYSCALL_CURSOR), "b"(SYSCALL_CURSOR_DISABLE));
		}
	}
}

namespace Time
{
	inline qword time()
	{
		qword result;
		asm("int $0x30"
			: "=a"(result)
			: "a"(SYSCALL_TIME), "b"(SYSCALL_TIME_GET));
		return result;
	}
	inline void sleep(ull ms)
	{
		asm("int $0x30"
			:
			: "a"(SYSCALL_TIME), "b"(SYSCALL_TIME_SLEEP), "D"(ms));
	}
}

namespace Keyboard
{
	inline KeyEvent getKeyEvent(bool blocking = true)
	{
		ushort result;
		asm("int $0x30"
			: "=a"(result)
			: "a"(SYSCALL_KEYBOARD), "b"(SYSCALL_KEYBOARD_KEYEVENT), "D"(blocking));
		return *(KeyEvent *)&result;
	}
	inline KeyEvent getKeyPressedEvent(bool blocking = true)
	{
		ushort result;
		asm volatile("int $0x30"
					 : "=a"(result)
					 : "a"(SYSCALL_KEYBOARD), "b"(SYSCALL_KEYBOARD_KEYPRESSEDEVENT), "D"(blocking));
		return *(KeyEvent *)&result;
	}
	inline KeyEvent getKeyReleasedEvent(bool blocking = true)
	{
		ushort result;
		asm("int $0x30"
			: "=a"(result)
			: "a"(SYSCALL_KEYBOARD), "b"(SYSCALL_KEYBOARD_KEYRELEASEDEVENT), "D"(blocking));
		return *(KeyEvent *)&result;
	}
}

namespace Scheduler
{
	inline int waitForThread(void *thread)
	{
		int returnValue;
		asm("int $0x30"
			: "=a"(returnValue)
			: "a"(SYSCALL_PROGENV), "b"(SYSCALL_PROGENV_WAITFORTHREAD), "D"(thread));
		return returnValue;
	}
	inline int waitForTask(void *task)
	{
		int returnValue;
		asm("int $0x30"
			: "=a"(returnValue)
			: "a"(SYSCALL_PROGENV), "b"(SYSCALL_PROGENV_WAITFORTASK), "D"(task));
		return returnValue;
	}
}

inline void exit(int exitCode)
{
	asm("int $0x30"
		:
		: "a"(SYSCALL_PROGENV), "b"(SYSCALL_PROGENV_EXIT), "D"(exitCode));
}