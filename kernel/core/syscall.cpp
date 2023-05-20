#include "../cpu/idt.h"
#include <iostream.h>
#include "../utils/time.h"
#include "scheduler.h"
#include "mem.h"
#include "../drivers/keyboard.h"

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
		// check that the task has access to the entire string
		if (!regs.cr3->getPhysicalAddress((qword)regs.rdi, physical))
		{
			// error
			cout << "Syscall error: address not mapped\n";
			return Scheduler::preempt(regs, Scheduler::preemptReason::taskExited);
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
		Keyboard::KeyEvent event = Keyboard::getKeyEvent_direct();
		if (event.keyCode == Keyboard::KeyCode::unknown && regs.rdi)
		{
			// no key in buffer, sleep
			regs.rip -= 2; // return to the interrupt instruction after sleep instead of after it
			Scheduler::waitForIrq(regs, IDT::Irq_no::ps2_keyboard);
		}
		else
			regs.rax = *(ushort *)&event;
	}
	break;
	case SYSCALL_KEYBOARD_KEYPRESSEDEVENT:
	{
		Keyboard::KeyEvent event = Keyboard::getKeyPressedEvent_direct();
		if (event.keyCode == Keyboard::KeyCode::unknown && regs.rdi)
		{
			// no key in buffer, sleep
			regs.rip -= 2; // return to the interrupt instruction after sleep instead of after it
			Scheduler::waitForIrq(regs, IDT::Irq_no::ps2_keyboard);
		}
		else
			regs.rax = *(ushort *)&event;
	}
	break;
	case SYSCALL_KEYBOARD_KEYRELEASEDEVENT:
	{
		Keyboard::KeyEvent event = Keyboard::getKeyReleasedEvent_direct();
		if (event.keyCode == Keyboard::KeyCode::unknown && regs.rdi)
		{
			// no key in buffer, sleep
			regs.rip -= 2; // return to the interrupt instruction after sleep instead of after it
			Scheduler::waitForIrq(regs, IDT::Irq_no::ps2_keyboard);
		}
		else
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
	case SYSCALL_TIME_SLEEP:
		return Scheduler::sleep(regs, regs.rdi + Time::time());
	}
}
void Syscall_ProgEnv(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_PROGENV_EXIT:
		cout << "A task is exiting with code " << (int)regs.rdi << '\n';
		return Scheduler::preempt(regs, Scheduler::preemptReason::taskExited);
	case SYSCALL_PROGENV_WAITFORTASK:
		return Scheduler::waitForTask(regs, (Task *)regs.rdi);
		/*case SYSCALL_PROGENV_ALLOCHEAP:
		{
			Memory::Allocate(0x3000, 0x1000);
			Memory::Allocate(0x10000, 0x1000);
			regs.cr3->mapRegion()
				regs.rax = (qword)Memory::Heap::build(0x800000000000, 0x10000);
			return;
		}*/
	}
}