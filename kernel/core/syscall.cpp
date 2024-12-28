#include "../utils/isriostream.h"
#include "../cpu/idt.h"
#include "../utils/time.h"
#include "scheduler.h"
#include "mem.h"

using namespace ISR::std;

extern "C" qword getRSP();
extern "C" qword getRBP();

void Syscall_Breakpoint(registers_t &);
void Syscall_Screen(registers_t &);
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
		return Keyboard::Syscall(regs);
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
		switch (/* Keyboard::driver_getKeyPressedEvent().getKeyCode() */ Keyboard::KeyCode::C)
		{
		case Keyboard::KeyCode::C:
			keepGoing = false;
			break;
		case Keyboard::KeyCode::S:
			constexpr int bytesPerRow = 16;
			// for (byte *i = (byte *)regs.rsp; i <= (byte *)regs.rbp; i += bytesPerRow)
			for (byte *i = (byte *)currRsp; i <= (byte *)regs.rbp; i += bytesPerRow)
				isr_displayMemoryRow(i);
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
		return Screen::driver_clear();
	case SYSCALL_SCREEN_PRINTSTR:
	{
		// get the physical address of the string from regs.rdi and the paging structures
		qword physical;
		bool user = (regs.cs & 0b11) == 0b11;
		// check that the task has access to the entire string
		if (!regs.cr3->getPhysicalAddress((qword)regs.rdi, physical, user))
		{
			// error
			cout << "Syscall error: address \"0x" << ::std::ostream::base::hex << regs.rdi << "\" not mapped\n";
			regs.rdi = (ull)-1;
			return Scheduler::preempt(regs, Scheduler::preemptReason::taskExited);
		}
		// for now, everything is identity mapped, no translation from physical to kernel virtual space
		// PageMapLevel4::getCurrent().mapRegion(...);
		return Screen::driver_print((const char *)physical);
	}
	case SYSCALL_SCREEN_PRINTCH:
		return Screen::driver_print((char)regs.rdi);
	case SYSCALL_SCREEN_PAINT:
		return Screen::driver_paint((byte)regs.rdi, (byte)regs.rsi, (Screen::Cell::Color)regs.rdx);
	}
}
void Syscall_Cursor(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_CURSOR_ENABLE:
		return Screen::Cursor::driver_enable(regs.rdi, regs.rsi);
	case SYSCALL_CURSOR_DISABLE:
		return Screen::Cursor::driver_disable();
	}
}
void Syscall_Time(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_TIME_GET:
		regs.rax = Time::driver_time();
		return;
	case SYSCALL_TIME_SLEEP:
		return Scheduler::sleep(regs, regs.rdi + Time::driver_time());
	}
}
void Syscall_ProgEnv(registers_t &regs)
{
	switch (regs.rbx)
	{
	case SYSCALL_PROGENV_EXIT:
		return Scheduler::preempt(regs, Scheduler::preemptReason::taskExited);
	case SYSCALL_PROGENV_WAITFORTASK:
		// if waitForThread fails, there is no way the return value
		// can be distinguished from the return value of the blocking thread
		return (void)Scheduler::waitForThread(regs, ((Task *)regs.rdi)->getMainThread());
	case SYSCALL_PROGENV_WAITFORTHREAD:
		// if waitForThread fails, there is no way the return value
		// can be distinguished from the return value of the blocking thread
		return (void)Scheduler::waitForThread(regs, (Thread *)regs.rdi);
	case SYSCALL_PROGENV_CREATETHREAD:
		return;
	}
}