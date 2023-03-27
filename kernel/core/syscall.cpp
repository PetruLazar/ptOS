#include "../cpu/idt.h"
#include <iostream.h>
#include "../utils/time.h"
#include "scheduler.h"

#define OMIT_FUNCS
#include <syscall.h>

using namespace std;

extern "C" qword getRSP();
extern "C" qword getRBP();

void Syscall_Breakpoint(registers_t &);
void Syscall_Screen(registers_t &);
void Syscall_Keyboard(registers_t &);
void Syscall_Filesystem(registers_t &);
void Syscall_Cursor(registers_t &);
void Syscall_Time(registers_t &);
void Syscall_ProgEnv(registers_t &);

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
	case SYSCALL_PROGENV:
		return Syscall_ProgEnv(regs);
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
		case Keyboard::KeyCode::C:
			keepGoing = false;
			break;
		case Keyboard::KeyCode::S:
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
	{
		// get the physical address of the string from regs.rdi and the paging structures
		qword physical;
		if (!regs.cr3->getPhysicalAddress((qword)regs.rdi, physical))
		{
			// error
			cout << "Syscall error: address not mapped\n";
			break;
		}
		// for now, everything is identity mapped, no translation from physical to kernel virtual space
		// PageMapLevel4::getCurrent().mapRegion(...);
		return Screen::print((const char *)physical);
	}
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
		return;
	}
}
void Syscall_ProgEnv(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_PROGENV_EXIT:
		cout << "A task is exiting with code " << (int)regs.rdi << '\n';
		return Scheduler::preempt(regs, true);
	}
}