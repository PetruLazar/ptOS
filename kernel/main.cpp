#include "cpu/idt.h"
#include "drivers/screen.h"
#include <string.h>
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
#include "core/explorer.h"
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

	Disk::Initialize();
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
	Disk::CleanUp();
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

	/*
	cli and sti is not needed in the interrupt handlers, since they are not trap gates
	test compatibility mode
	implement syscall maybe?
	change video mode
	shutdown
	c++ exception handling maybe?
	apic
	sound
	dynamic linking
	maybe support for pe executables
	networking
	*/
}

void readCommand(string &cmd)
{
	while (true)
	{
		Keyboard::KeyEvent event = Keyboard::getKeyPressedEvent();
		switch (event.getKeyCode())
		{
		case Keyboard::KeyCode::arrowUp:
			Screen::scrollUp();
			break;
		case Keyboard::KeyCode::arrowDown:
			Screen::scrollDown();
			break;
		default:
		{
			char ch = event.getChar();
			switch (ch)
			{
			case '\b': // delete one char
				if (cmd[0] == 0)
					break;
				cout << "\b \b";
				cmd.pop_back();
				break;
			case '\r': // finish reading
				cout << '\n';
				return;
			case 0:
				break;
			default:
				cout << ch;
				cmd.push_back(ch);
			}
		}
		}
	}
}
bool getSubcommand(string &cmd, string &subCmd)
{
	ull space = cmd.firstOf(' ');
	if (space == string::npos)
	{
		// no space
		if (cmd.length() == 0)
			return false;
		subCmd = cmd;
		cmd.erase();
		return true;
	}
	subCmd.assign(cmd.data(), space);
	cmd.erase(0, space + 1);
	return true;
}

void terminal()
{
	cout << "Welcome to ptOS!\n";

	bool keepRunning = true;
	string cmd, subCmd;

	while (keepRunning)
	{
		// read command
		readCommand(cmd);

		if (!getSubcommand(cmd, subCmd))
			continue;

		if (subCmd == "exit")
		{
			keepRunning = false;
		}
		else if (subCmd == "start" || subCmd == "call")
		{
			// convert string argument to string16
			string16 filename;
			for (char c : cmd)
				filename += c;

			bool found = false;
			for (auto &disk : *Disk::devices)
			{
				for (auto &part : disk->partitions)
				{
					Task *task = Task::createTask((char16_t)part->letter + string16(u":/programs/") + filename + u".bin");
					if (task)
					{
						Scheduler::add(task);
						if (subCmd == "call")
							Scheduler::waitForTask(task);
						found = true;
						break;
					}
				}
				if (found)
					break;
			}
			if (!found)
				cout << "Failed to run program\n";
		}
		else if (subCmd == "dump")
		{
			// get optional arguments
			bool forceText = false,
				 forceBin = false;
			while (cmd[0] == '-' || cmd[0] == '/')
			{
				getSubcommand(cmd, subCmd);
				switch (subCmd[1])
				{
				case 'b': // bainry mode
					if (forceText)
					{
						cout << "Conflicting argument 'b', forcing binary mode instead of text mode.\n";
						forceText = false;
					}
					else if (forceBin)
						cout << "Redundant argument 'b', already in forced binary mode.\n";
					forceBin = true;
					break;
				case 't':
				case 'a': // text (or ascii) mode
					if (forceBin)
					{
						cout << "Conflicting argument '" << subCmd[1] << "', forcing text mode instead of binary mode.\n";
						forceBin = false;
					}
					else if (forceText)
						cout << "Redundant argument '" << subCmd[1] << "', already in forced text mode.\n";
					forceText = true;
					break;
				}
			}

			// convert string argument to string16
			string16 filename;
			for (char c : cmd)
				filename += c;

			byte *content;
			ull length;
			Filesystem::result res = Filesystem::ReadFile(filename, content, length);
			if (res == Filesystem::result::success)
			{
				// if no mode is forced, determine the type of the contents
				if (!(forceBin || forceText))
				{
					ull checkLen = length > 0x1000 ? 0x1000 : length; // if file is longer than 4KB, check only the first 4KB
					forceText = true;								  // assume text
					for (ull i = 0; i < checkLen; i++)
					{
						byte b = content[i];
						if (b == 0x00 || b == 0xff) // assume the file is binary
						{
							forceText = false;
							forceBin = true;
							break;
						}
					}
				}

				if (forceText)
				{
					cout << string((char *)content, length) << '\n';
				}
				else
				{
					DisplyMemoryBlock(content, length);
				}
				delete[] content;
			}
			else
				cout << "Error: " << Filesystem::resultAsString(res) << '\n';
		}
		else if (subCmd == "edit")
		{
		}
		else if (subCmd == "debug")
		{
			cout << "Function to debug is at " << (void *)((char *)main - 0x8000 + 0x200) << '\n';
		}
		else if (subCmd == "pci")
		{
			PCI::EnumerateDevices();
			cout << "Done!\n";
		}
		else if (subCmd == "clear")
		{
			Screen::clear();
		}
		else if (subCmd == "memmap")
		{
			Memory::DisplayMap();
		}
		else if (subCmd == "apic")
		{
			cout << "Apic ";
			if (!PIC::detectApic())
				cout << "not ";
			cout << "detected\n";
		}
		else if (subCmd == "cpu")
		{
			if (cmd == "speed")
			{
				ull clk = clock();
				Time::sleep(1000);
				ull diff = clock() - clk;
				cout << "Average: " << diff << '\n';
			}
			else if (cmd == "vendor")
			{
				char processorVendorString[13];
				dword unused;
				cpuid(0, unused, (dword &)processorVendorString[0], (dword &)processorVendorString[8], (dword &)processorVendorString[4]);
				processorVendorString[12] = 0;
				cout << "Processor vendor string is: " << processorVendorString << '\n';
			}
			else
			{
				cout << "Invalid command.\n";
			}
		}
		else if (subCmd == "clock")
		{
			qword clocks = clock();

			char str[20];
			qwordToHexString(str, clocks);
			cout << "Clock is: " << str << '\n';
		}
		else if (subCmd == "fsPart")
		{
			Filesystem::displayPartitions();
		}
		else if (subCmd == "diskpart")
		{
			int c = 0;
			for (auto &disk : *Disk::devices)
			{
				cout << "Disk " << c++ << ":\n";
				for (auto &part : disk->partitions)
				{
					cout << '\t' << toUpper(part->letter) << ": " << part->lbaLen << " sectors at " << part->lbaStart << ", " << part->type() << " partition\n";
				}
			}
		}
		else if (subCmd == "disk")
		{
			int c = 0;
			cout << "Currently " << Disk::devices->getSize() << " disk drives:";

			for (auto *dev : *Disk::devices)
			{
				ull size = dev->getSize() * 512; // assume 512 sector size
				byte magnitude = 0;
				while (size >= 1024 && magnitude < 4)
				{
					// round to nearest 1024 multiple
					size = ((size - 512) & ~(ull)0x3ff) + 1024;
					size /= 1024;
					magnitude++;
				}
				char unit = '?';
				switch (magnitude)
				{
				case 0:
					unit = 'B';
					break;
				case 1:
					unit = 'K';
					break;
				case 2:
					unit = 'M';
					break;
				case 3:
					unit = 'G';
					break;
				case 4:
					unit = 'T';
				}

				cout << "\n\nDisk " << c++
					 << ":\n\tModel: " << dev->getModel()
					 << "\n\tSize: " << size << unit << " (" << dev->getSize() << " sectors)\n"
																				  "\tLocation: "
					 << dev->getLocation();
			}
			cout << '\n';
		}
		else if (subCmd == "explorer")
		{
			if (cmd.length() != 1)
				cout << "Partition must be a single letter\n";
			else
				Explorer::Start(cmd[0]);
		}
		else if (subCmd == "test")
		{
			// convert to number
			ull i = 0;
			for (char c : cmd)
			{
				if (c < '0' || c > '9')
				{
					cmd.erase();
					break;
				}
				i = i * 10 + (c - '0');
			}
			if (cmd.length() == 0)
				cout << "Invalid command.\n";
			else
			{
				switch (i)
				{
				case 0:
				{
					string test = Memory::getStringMemoryMap();
					// Filesystem::CreateFile
					Filesystem::result res = Filesystem::WriteFile(u"c:/export.txt", (byte *)test.data(), test.length());
					if (res != Filesystem::result::success)
					{
						if (res == Filesystem::result::fileDoesNotExist)
						{
							if (Filesystem::CreateFile(u"c:/export.txt", (byte *)test.data(), test.length()) != Filesystem::result::success)
							{
								cout << "Could not create file.\n";
							}
							else
								cout << "Success\n";
						}
						else
							cout << "Write failed: " << Filesystem::resultAsString(res) << '\n';
					}
					else
						cout << "Success\n";
					break;
				}
				case 1:
				{
					Task *task = Task::createTask(u"C:/programs/hello1.bin");
					if (task)
						Scheduler::add(task);
					task = Task::createTask(u"C:/programs/hello2.bin");
					if (task)
						Scheduler::add(task);
					break;
				}
				default:
					cout << "Invalid test number.\n";
				}
			}
		}
		else
		{
			cout << "Invalid command.\n";
		}

		/*switch (key)
		{
		case Keyboard::KeyCode::alpha3:
		case Keyboard::KeyCode::numpad_3:
		{
		}
		break;
		case Keyboard::KeyCode::alpha7:
		case Keyboard::KeyCode::numpad_7:
		{
			// test allocations, one at a time
			break;
		}
		case Keyboard::KeyCode::alpha8:
		case Keyboard::KeyCode::numpad_8:
		{
			// test stressing the allocator
			break;
		}
		}*/

		cmd.erase();
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