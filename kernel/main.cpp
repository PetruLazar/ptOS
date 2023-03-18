#include "cpu/idt.h"
#include "drivers/screen.h"
#include "utils/string.h"
#include "cpu/pic.h"
#include "cpu/ports.h"
#include "drivers/keyboard.h"
#include "utils/iostream.h"
#include "utils/time.h"
// #include "../libc/rand.h"
#include "core/mem.h"
#include "cpu/cpuid.h"
#include "cpu/gdt.h"
#include "drivers/pci.h"
#include "core/filesystem.h"
#include "core/sys.h"
#include "utils/math.h"
#include "core/paging.h"
using namespace std;

#define QEMU

int waitTime = 0x7fffff;
void delay()
{
	for (int i = 0; i < waitTime; i++)
		;
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

	// Random::setSeed(clock() & 0xffffffff);

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
		{
			byte *content;
			ull len;
			if (!Filesystem::ReadFile(u"C:/snake/snake.bin", content, len))
			{
				cout << "Could not read file\n";
				break;
			}
			byte *pageSpace = (byte *)Memory::Allocate(0x7000, 0x1000);
			PageMapLevel4 *paging = (PageMapLevel4 *)pageSpace;
			paging->clearAll();
			qword freeSpace = (qword)(paging + 0x1);
			paging->mapRegion(freeSpace, 0x1000, 0x1000, 0x4f000);									// page kernel and stack
			paging->mapRegion(freeSpace, 0x100000, (qword)content, alignValueUpwards(len, 0x1000)); // page loaded code

			if (freeSpace > (qword)(pageSpace + 0x7000))
			{
				cout << "Ran out of space for the paging structure.\n";
				delete[] pageSpace;
				delete[] content;
				break;
			}
			PageMapLevel4 &kernelPaging = PageMapLevel4::getCurrent(); // get current kernel paging
			paging->setAsCurrent();									   // apply program paging

			voidf prog = (voidf)(0x100000);
			prog();						 // call the loaded code
			kernelPaging.setAsCurrent(); // back to the original paging

			// clean-up
			delete[] pageSpace;
			delete[] content;
		}
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
			// Explorer::Start();
			cout << "The file explorer is not currently available...\n";
			break;

			// Filesystem::DirectoryIterator *list = Filesystem::GetDirectoryIterator(u"C:/FOLDER1/FOLDER4");
			Filesystem::DirectoryIterator *list = Filesystem::GetDirectoryIterator(u"C:/");

			if (list == nullptr)
			{
				cout << "Iterator was nullptr\n";
				break;
			}

			while (!list->finished())
			{
				cout << list->getString() << " (" << ostream::base::dec << list->getSize() << " bytes)\n";
				list->advance();
			}

			delete list;
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha1:
		case Keyboard::KeyEvent::KeyCode::numpad_1:
		{
			break;
			/*basic_string<byte *> allocations;
			for (int i = 0; i < 100; i++)
			{
				switch (Random::get() % 3)
				{
				case 0:
				case 2:
				{
					ull size = Random::get() % 10000 + 1;
					byte *block = (byte *)Memory::Allocate(size, 0x1000);
					allocations.push_back(block);
					cout << "Allocated " << size << " bytes at " << block << '\n';
					break;
				}
				case 1:
				{
					ull len = allocations.length();
					if (!len)
					{
						cout << "No allocations to delete\n";
						break;
					}
					ull index = Random::get() % len;
					byte *block = allocations[index];
					delete[] block;
					allocations.erase(index);
					cout << "Deallocated block at " << block << '\n';
					break;
				}
				}

				System::pause(false);
			}

			for (auto &block : allocations)
			{
				delete[] block;
				cout << "Deallocated block at " << block << '\n';
			}

			cout << '\n'; */
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha2:
		case Keyboard::KeyEvent::KeyCode::numpad_2:
		{
			while (true)
			{
				uint nr, align;
				cin >> nr;
				if (!nr)
					break;
				cin >> align;
				cout << "result: " << alignValueUpwards(nr, align) << '\n';
			}
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
		{
			ull len;
			byte *contents;

			// read before
			if (Filesystem::ReadFile(u"C:/FOLDER1/FOLDER4/FILE7.TXT", contents, len))
			{
				string str((char *)contents, len);
				cout << "Contents before:\n"
					 << str << '\n';
				delete[] contents;
			}
			else
			{
				cout << "Failed to read file\n";
				break;
			}

			// write
			{
				string str = "These are the new contents of the file with the full path \"C:/FOLDER1/FOLDER4/FILE7.TXT\".\n"
							 "It is intentionally longer than the initial contents, so that the function is properly tested.\n"
							 "Also, it contains more lines than the initial file.\n\n"
							 "Hello mom! ... and the rest of the world to... sure... why not...?\n";

				if (!Filesystem::WriteFile(u"C:/FOLDER1/FOLDER4/FILE7.TXT", (byte *)str.data(), str.getSize()))
				{
					cout << "Failed to write to file\n";
					break;
				}
			}

			// read after
			if (Filesystem::ReadFile(u"C:/FOLDER1/FOLDER4/FILE7.TXT", contents, len))
			{
				string str((char *)contents, len);
				cout << "Contents after:\n"
					 << str << '\n';
				delete[] contents;
			}
			else
			{
				cout << "Failed to read file\n";
				break;
			}
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha5:
		case Keyboard::KeyEvent::KeyCode::numpad_5:
		{
			// if (Filesystem::ReadFile(u"C:/aa b.txt", contents, len))

			ull len;
			byte *contents;
			if (Filesystem::ReadFile(u"C:/New Text Document.txt", contents, len))
			{
				// DisplyMemoryBlock()
				cout << "Size of file: " << len << " bytes\n";
				string str((char *)contents, len);
				cout << str << '\n';
				delete[] contents;
			}
			else
				cout << "Failed to read file\n";
		}
		break;
		case Keyboard::KeyEvent::KeyCode::alpha6:
		case Keyboard::KeyEvent::KeyCode::numpad_6:
		{
			// string contents = "Might not seem like it, but this file is a big milestone for PTOS. This file has been entirely created and written to in the PTOS' FAT32 Filesystem Driver.";
			// cout << contents << '\n';

			// if (Filesystem::WriteFile(u"C:/New Text Document.txt", (byte *)contents.data(), contents.getSize()))
			// 	cout << "Write complete!\n";
			// else
			// 	cout << "Write failed!\n";
		}
		break;
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