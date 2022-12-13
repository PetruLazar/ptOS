#include "cpu/idt.h"
#include "drivers/screen.h"
#include "utils/string.h"
#include "cpu/pic.h"
#include "cpu/ports.h"
#include "drivers/keyboard.h"
#include "utils/iostream.h"
#include "utils/time.h"
#include "games/snake.h"
#include "utils/rand.h"
#include "core/mem.h"
#include "cpu/cpuid.h"
#include "cpu/gdt.h"
#include "drivers/pci.h"
#include "core/filesystem.h"
#include "core/sys.h"

using namespace std;

#define QEMU

int waitTime = 0x7fffff;
void delay()
{
	for (int i = 0; i < waitTime; i++)
		;
}

class Test
{
	ull x, y;

public:
	Test() { cout << "+\n"; }
	Test(const Test &t) { cout << "+\n"; }
	Test(Test &&t) { cout << "+\n"; }
	~Test() { cout << "-\n"; }
};
Test test()
{
	return Test();
}

// calling convention: 	rdi, rsi, rdx, rcx, r8, r9, stack...
//						xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, stack...

extern "C" void testint();
extern "C" void makesyscall();

extern "C" void testCompMode();

// void *memoryToFileOffset(void *input) { return (byte *)input - 0x8000 + 0x200; }

extern void main()
{
	Memory::Initialize();

	Screen::Initialize();

	IDT::Initialize();

	Time::Initialize();
	Keyboard::Initialize();
	enableInterrupts();

	GDT::Initialize();

	cout << "Welcome to ptOS!\n";
	// testint();

	// makesyscall();

	Random::setSeed(clock() & 0xffffffff);

	Filesystem::Initialize();
	PCI::InitializeDevices();

	// error is at 9928
	// cout << memoryToFileOffset(Keyboard::KeyEvent::getKeyCode) << '\n';

	ull initialAllocCount = Memory::Heap::getAllocationCountFromSelected();
	bool keepRunning = true;
	while (keepRunning)
	{
		// check for keyboard input
		/*char ch = Keyboard::getKeyPressedEvent().getChar();
		if (ch)
			cout << "Pressed " << ch << " (" << (int)ch << ")\n";
		continue;*/

		Keyboard::KeyEvent::KeyCode key = Keyboard::getKeyPressedEvent().getKeyCode();

		switch (key)
		{
		case Keyboard::KeyEvent::KeyCode::Escape:
			keepRunning = false;
			break;
		case Keyboard::KeyEvent::KeyCode::I:
			asm("int $0x30");
			break;
		case Keyboard::KeyEvent::KeyCode::S:
			Snake::Play();
			break;
		case Keyboard::KeyEvent::KeyCode::arrowUp:
			Screen::scrollUp();
			break;
		case Keyboard::KeyEvent::KeyCode::arrowDown:
			Screen::scrollDown();
			break;
		case Keyboard::KeyEvent::KeyCode::C:
			Screen::clear();
			break;
		case Keyboard::KeyEvent::KeyCode::D:
			cout << "Function to debug is at " << (void *)((char *)Disk::Initialize - 0x8000 + 0x200) << '\n';
			break;
		case Keyboard::KeyEvent::KeyCode::A:
			cout << "Apic ";
			if (!PIC::detectApic())
				cout << "not ";
			cout << "detected\n";
			break;
		case Keyboard::KeyEvent::KeyCode::M:
			Memory::DisplayMap();
			break;
		case Keyboard::KeyEvent::KeyCode::L:
			PCI::EnumerateDevices();
			cout << "Done!\n";
			break;
		case Keyboard::KeyEvent::KeyCode::V:
		{
			char processorVendorString[13];
			dword unused;
			cpuid(0, unused, (dword &)processorVendorString[0], (dword &)processorVendorString[8], (dword &)processorVendorString[4]);
			processorVendorString[12] = 0;
			cout << "Processor vendor string is: " << processorVendorString << '\n';
		}
		break;
		case Keyboard::KeyEvent::KeyCode::T:
		{
			qword clocks = clock();

			char str[20];
			qwordToHexString(str, clocks);
			cout << "Clock is: " << str << '\n';
		}
		break;

		case Keyboard::KeyEvent::KeyCode::alpha0:
		case Keyboard::KeyEvent::KeyCode::numpad_0:
		{
			
			Filesystem::listDirectoryEntries("C");
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha1:
		case Keyboard::KeyEvent::KeyCode::numpad_1:
		{
			byte *buffer = new byte[512], err;
			for (int i = 0; i < 512; i++)
				buffer[i] = Random::get() & 0xff;
			if (err = Disk::accessATAdrive(Disk::accessDir::read, 0, 0x0, 1, buffer))
			{
				Disk::displayError(0, err);
				break;
			}
			for (int i = 0; i < 512; i += 16)
				displayMemoryRow(buffer + i);
			delete[] buffer;
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha2:
		case Keyboard::KeyEvent::KeyCode::numpad_2:
		{
			Disk::Device *dev = Disk::devices;
			cout << dev[0].reserved << ' '
				 << dev[1].reserved << ' '
				 << dev[2].reserved << ' '
				 << dev[3].reserved << '\n';
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha3:
		case Keyboard::KeyEvent::KeyCode::numpad_3:
		{
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
		case Keyboard::KeyEvent::KeyCode::alpha4:
		case Keyboard::KeyEvent::KeyCode::numpad_4:
			Filesystem::listDirectoryEntries("C");
			break;
		case Keyboard::KeyEvent::KeyCode::alpha5:
		case Keyboard::KeyEvent::KeyCode::numpad_5:
		{
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha6:
		case Keyboard::KeyEvent::KeyCode::numpad_6:
		case Keyboard::KeyEvent::KeyCode::alpha7:
		case Keyboard::KeyEvent::KeyCode::numpad_7:
		case Keyboard::KeyEvent::KeyCode::alpha8:
		case Keyboard::KeyEvent::KeyCode::numpad_8:
		case Keyboard::KeyEvent::KeyCode::alpha9:
		case Keyboard::KeyEvent::KeyCode::numpad_9:
			cout << "This test is unused\n";
			break;
		}
	}

	// some cleanup
	// Screen::Cleanup();

	ull currAllocCount = Memory::Heap::getAllocationCountFromSelected();
	if (initialAllocCount != currAllocCount)
	{
		cout << "Memory leaks detected: " << currAllocCount - initialAllocCount << " allocations!\n";
		System::pause();
	}

#ifdef QEMU
	cout << "Shutting down...";
	disableInterrupts();
	outw(0x604, 0x2000);
#else
	cout << "Shutdown not implemented...\n";
#endif

	// disk operations

	// cli and sti is not needed in the interrupt handlers, since they are not trap gates

	// move gdt to c++
	// add 4k-aligned allocation functions

	// filesystem

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