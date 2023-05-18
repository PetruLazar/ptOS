#include "cpu/idt.h"
#include "drivers/screen.h"
#include "utils/string.h"
#include "cpu/pic.h"
#include "cpu/ports.h"
#include "drivers/keyboard.h"
#include <iostream.h>
#include "utils/time.h"
// #include "../libc/rand.h"
#include "core/mem.h"
#include "cpu/cpuid.h"
#include "cpu/gdt.h"
#include "drivers/pci.h"
#include "core/filesystem.h"
#include "core/sys.h"
#include <math.h>
#include "core/paging.h"
#include "core/scheduler.h"
#define OMIT_FUNCS
#include <syscall.h>
using namespace std;

#define QEMU

// calling convention: 	rdi, rsi, rdx, rcx, r8, r9, stack...
//						xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, stack...

extern "C" void testint();
extern "C" void makesyscall();

extern "C" qword getRFlags();
extern "C" void testCompMode();

// void *memoryToFileOffset(void *input) { return (byte *)input - 0x8000 + 0x200; }

void terminal();
void shutdown();

extern void main()
{
	Memory::Initialize();

	Screen::Initialize();

	IDT::Initialize();

	Time::Initialize();
	Keyboard::Initialize();

	GDT::Initialize();

	Filesystem::Initialize();
	PCI::InitializeDevices();

	// get terminal task ready and initialize scheduler
	Task *terminalTask = new Task(registers_t(), true);
	Scheduler::Initialize(terminalTask);

	enableInterrupts();

	terminal();

	disableInterrupts();

	Scheduler::CleanUp();
	delete terminalTask;
	Filesystem::CleanUp();
	Keyboard::CleanUp();
	Screen::Cleanup();

	ull currAllocCount = Memory::Heap::getAllocationCountFromSelected();
	if (currAllocCount)
	{
		// screen and keyboard do not need re-initialization, since the pointers preserved their values, and they will safely use the memory
		// previously allocated to them, which will be free since no other memory allocations are used

		cout << "Memory leaks detected: " << currAllocCount << " allocations!\n";
		Memory::Heap::displayAllocationSummaryFromSelected();
		enableInterrupts();
		System::pause();
		disableInterrupts();
	}

	shutdown();

	// disk operations

	// cli and sti is not needed in the interrupt handlers, since they are not trap gates

	// test compatibility mode

	// pci supoprt

	// implement syscall

	// change video mode

	// test dpl changing

	// shutdown

	// c++ exception handling maybe?

	// apic

	// sound

	// loading of user programs, with dynamic linking
	// maybe support for pe executables

	// multitasking

	// networking
}

void terminal()
{
	cout << "Welcome to ptOS!\n";

	bool keepRunning = true;
	while (keepRunning)
	{
		// check for keyboard input
		/*char ch = Keyboard::getKeyPressedEvent().getChar();
		if (ch)
			cout << "Pressed " << ch << " (" << (int)ch << ")\n";
		continue;*/
		Keyboard::KeyCode key = Keyboard::getKeyPressedEvent().getKeyCode();

		switch (key)
		{
		case Keyboard::KeyCode::Escape:
			keepRunning = false;
			break;
		case Keyboard::KeyCode::I:
			// asm("int $0x30");
			break;
		case Keyboard::KeyCode::S:
		{
			Task *task = Task::createTask(u"C:/programs/snake.bin");
			if (task)
			{
				Scheduler::add(task);
				Scheduler::waitForTask(task);
			}
			break;
		}
		case Keyboard::KeyCode::arrowUp:
			Screen::scrollUp();
			break;
		case Keyboard::KeyCode::arrowDown:
			Screen::scrollDown();
			break;
		case Keyboard::KeyCode::C:
			Screen::clear();
			break;
		case Keyboard::KeyCode::D:
		{
			cout << "Function to debug is at " << (void *)((char *)Keyboard::getKeyEvent_direct - 0x8000 + 0x200) << '\n';
			// cout << "Function to debug is at " << (void *)((char *)Disk::Initialize - 0x8000 + 0x200) << '\n';
			break;
		}
		case Keyboard::KeyCode::A:
			cout << "Apic ";
			if (!PIC::detectApic())
				cout << "not ";
			cout << "detected\n";
			break;
		case Keyboard::KeyCode::M:
			Memory::DisplayMap();
			break;
		case Keyboard::KeyCode::L:
			PCI::EnumerateDevices();
			cout << "Done!\n";
			break;
		case Keyboard::KeyCode::V:
		{
			char processorVendorString[13];
			dword unused;
			cpuid(0, unused, (dword &)processorVendorString[0], (dword &)processorVendorString[8], (dword &)processorVendorString[4]);
			processorVendorString[12] = 0;
			cout << "Processor vendor string is: " << processorVendorString << '\n';
		}
		break;
		case Keyboard::KeyCode::T:
		{
			qword clocks = clock();

			char str[20];
			qwordToHexString(str, clocks);
			cout << "Clock is: " << str << '\n';
		}
		break;

		case Keyboard::KeyCode::alpha0:
		case Keyboard::KeyCode::numpad_0:
		{
			// Explorer::Start();
			cout << "The file explorer is not currently available...\n";
			break;
		}
		break;
		case Keyboard::KeyCode::alpha1:
		case Keyboard::KeyCode::numpad_1:
		{
			asm("int $0x30"
				:
				: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_PRINTSTR), "D"("Hello world!\n"));
		}
		break;
		case Keyboard::KeyCode::alpha2:
		case Keyboard::KeyCode::numpad_2:
		{
			asm("int $0x30"
				:
				: "a"(SYSCALL_SCREEN), "b"(SYSCALL_SCREEN_PRINTSTR), "D"(0x0000));
		}
		break;
		case Keyboard::KeyCode::alpha3:
		case Keyboard::KeyCode::numpad_3:
		{
			*(byte *)(0x00) = 5;
			break;
			bool first = true;
			for (int i = 0; i < 4; i++)
			{
				Disk::Device &disk = Disk::devices[i];
				if (disk.reserved)
				{
					if (first)
						first = false;
					else
						System::pause(false);
					cout << "Disk " << i + 1 << ":\n"
												"   channel: "
						 << ostream::base::hex << disk.channel
						 << "\n   drive: " << disk.drive
						 << "\n   type: " << Disk::deviceTypes[(byte)disk.type]
						 << "\n   signature: " << disk.signature
						 << "\n   capabilities: " << disk.capabilities
						 << "\n   command sets: " << disk.commandSets
						 /*<< "\n   max lba: " << getIdField(identificationSpace, IDfield::maxLba)
						 << "\n   max lba ext: " << getIdField(identificationSpace, IDfield::maxLbaExt)*/
						 << "\n   size: " << disk.size
						 << "\n   model: " << disk.model << ostream::base::dec << "\n\n";
				}
			}
			cout << "Done!\n";
		}
		break;
		case Keyboard::KeyCode::alpha4:
		case Keyboard::KeyCode::numpad_4:
		{
			Task *task = Task::createTask(u"c:/programs/test1.bin");
			if (task)
			{
				Scheduler::add(task);
				Scheduler::waitForTask(task);
			}
			break;
		}
		case Keyboard::KeyCode::alpha5:
		case Keyboard::KeyCode::numpad_5:
		{
			Task *task = Task::createTask(u"c:/programs/hello2.bin");
			if (task)
				Scheduler::add(task);
			break;
		}
		case Keyboard::KeyCode::alpha6:
		case Keyboard::KeyCode::numpad_6:
		{
			uint count;
			cout << "how many? ";
			cin >> count;
			for (uint i = 0; i < count; i++)
			{
				Task *task = Task::createTask(i & 1 ? u"c:/programs/hello2.bin" : u"c:/programs/hello1.bin");
				if (task)
					Scheduler::add(task);
			}
			break;
		}
		case Keyboard::KeyCode::alpha7:
		case Keyboard::KeyCode::numpad_7:
			Time::sleep(1000);
			break;
		case Keyboard::KeyCode::alpha8:
		case Keyboard::KeyCode::numpad_8:
		case Keyboard::KeyCode::alpha9:
		case Keyboard::KeyCode::numpad_9:
			cout << "This test is unused\n";
			break;
		}
	}

	// some cleanup
	// Screen::Cleanup();
}

void shutdown()
{
	disableInterrupts();

#ifdef QEMU
	// cout << "Shutting down...";
	outw(0x604, 0x2000);
#else
	cout << "Shutdown not implemented...\n";
#endif
	while (true)
		;
}