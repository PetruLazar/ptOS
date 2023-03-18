#include "../cpu/idt.h"
#include "../utils/iostream.h"
#include "../utils/time.h"

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

#define SYSCALL_CURSOR_ENABLE 0
#define SYSCALL_CURSOR_DISABLE 1

#define SYSCALL_TIME_GET 0

using namespace std;

extern "C" qword getRSP();
extern "C" qword getRBP();

void Syscall_Breakpoint(registers_t &);
void Syscall_Screen(registers_t &);
void Syscall_Keyboard(registers_t &);
void Syscall_Filesystem(registers_t &);
void Syscall_Cursor(registers_t &);
void Syscall_Time(registers_t &);

extern "C" void os_serviceHandler(registers_t &regs)
{
	switch (regs.rax)
	{
	case SYSCALL_BREAKPOINT:
		return Syscall_Breakpoint(regs);
	case SYSCALL_SCREEN:
		return Syscall_Screen(regs);
	case SYSCALL_KEYBOARD:
		return Syscall_Keyboard(regs);
	case SYSCALL_FILESYSTEM:
		break;
	case SYSCALL_CURSOR:
		return Syscall_Cursor(regs);
	case SYSCALL_TIME:
		return Syscall_Time(regs);
	}
}

void Syscall_Breakpoint(registers_t &regs)
{
	qword currRsp = getRBP();
	cout << "Breakpoint reached\n";
	bool keepGoing = true;
	enableInterrupts();
	while (keepGoing)
	{
		switch (Keyboard::getKeyPressedEvent().getKeyCode())
		{
		case Keyboard::KeyEvent::KeyCode::C:
			keepGoing = false;
			break;
		case Keyboard::KeyEvent::KeyCode::S:
			constexpr int bytesPerRow = 16;
			// for (byte *i = (byte *)regs.rsp; i <= (byte *)regs.rbp; i += bytesPerRow)
			for (byte *i = (byte *)currRsp; i <= (byte *)regs.rbp; i += bytesPerRow)
				displayMemoryRow(i);
			cout << "RBP: " << (void *)regs.rbp << "   RSP: " << (void *)regs.rsp << "   Current RSP: " << (void *)currRsp << '\n';
			break;
		}
	}
	disableInterrupts();
}
void Syscall_Screen(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_SCREEN_CLEAR:
		return Screen::clear();
	case SYSCALL_SCREEN_PRINTSTR:
		// get the virtual address of the string from regs.rdi and the paging structures
		//  return Screen::print((const char *)regs.rdi);
		break;
	case SYSCALL_SCREEN_PRINTCH:
		return Screen::print((char)regs.rdi);
	case SYSCALL_SCREEN_PAINT:
		return Screen::paint((byte)regs.rdi, (byte)regs.rsi, (Screen::Cell::Color)regs.rdx);
	}
}
void Syscall_Keyboard(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_KEYBOARD_KEYEVENT:
	{
		Keyboard::KeyEvent event = Keyboard::getKeyEvent();
		regs.rax = *(ushort *)&event;
	}
	break;
	case SYSCALL_KEYBOARD_KEYPRESSEDEVENT:
	{
		Keyboard::KeyEvent event = Keyboard::getKeyPressedEvent();
		regs.rax = *(ushort *)&event;
	}
	break;
	case SYSCALL_KEYBOARD_KEYRELEASEDEVENT:
	{
		Keyboard::KeyEvent event = Keyboard::getKeyReleasedEvent();
		regs.rax = *(ushort *)&event;
	}
	break;
	}
}
void Syscall_Cursor(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_CURSOR_ENABLE:
		return Screen::Cursor::enable(regs.rdi, regs.rsi);
	case SYSCALL_CURSOR_DISABLE:
		return Screen::Cursor::disable();
	}
}
void Syscall_Time(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_TIME_GET:
		regs.rax = Time::time();
		break;
	}
}