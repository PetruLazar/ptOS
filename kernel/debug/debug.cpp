#include "debug.h"
#include <keyboard.h>
#include "../utils/isriostream.h"
#include "../core/scheduler.h"

namespace Debug
{
	void readCommand(registers_t &regs, std::string &cmd)
	{
		while (true)
		{
			// disableInterrupts();
			// asm volatile("jmp $");
			// asm volatile("int 0x80");
			Keyboard::KeyEvent event = Keyboard::getKeyPressedEvent(false);
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
					ISR::std::cout << "\b \b";
					cmd.pop_back();
					break;
				case '\r': // finish reading
					ISR::std::cout << '\n';
					return;
				case 0:
					break;
				default:
					ISR::std::cout << ch;
					cmd.push_back(ch);
				}
			}
			}
		}
	}
	bool getSubcommand(std::string &cmd, std::string &subCmd)
	{
		ull space = cmd.firstOf(' ');
		if (space == std::string::npos)
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

	bool interpretHexStr(const std::string &str, ull &result)
	{
		result = 0;
		for (char c : str)
		{
			switch (c)
			{
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					result = (result << 4) | (c - '0');
					break;
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
					result = (result << 4) | (c - 'A' + 10);
					break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'e':
				case 'f':
					result = (result << 4) | (c - 'a' + 10);
					break;
				default:
					return false;
			}
		}

		return true;
	}

	void DebugExceptionHandler(registers_t &regs, qword int_no)
	{
		ISR::std::cout << "Breakpoint reached at " << (void*)regs.rip << '\n';
		// disable scheduling and enable interrupts
		Scheduler::disable();
		enableInterrupts();

		bool exit_handler = false;
		while (!exit_handler)
		{
			// read command
			std::string cmd, subCmd;
			readCommand(regs, cmd);

			// precess command
			while (getSubcommand(cmd, subCmd) && !exit_handler)
			{
				if (subCmd == "rax")
				{
					ISR::std::cout << "rax = " << (void*)regs.rax << '\n';
				}
				else if (subCmd == "rbx")
				{
					ISR::std::cout << "rbx = " << (void*)regs.rbx << '\n';
				}
				else if (subCmd == "rcx")
				{
					ISR::std::cout << "rcx = " << (void*)regs.rcx << '\n';
				}
				else if (subCmd == "rdx")
				{
					ISR::std::cout << "rdx = " << (void*)regs.rdx << '\n';
				}
				else if (subCmd == "rdi")
				{
					ISR::std::cout << "rdi = " << (void*)regs.rdi << '\n';
				}
				else if (subCmd == "rsi")
				{
					ISR::std::cout << "rsi = " << (void*)regs.rsi << '\n';
				}
				else if (subCmd == "rsp")
				{
					ISR::std::cout << "rsp = " << (void*)regs.rsp << '\n';
				}
				else if (subCmd == "rbp")
				{
					ISR::std::cout << "rbp = " << (void*)regs.rbp << '\n';
				}
				else if (subCmd == "r9")
				{
					ISR::std::cout << "r9 = " << (void*)regs.r9 << '\n';
				}
				else if (subCmd == "r10")
				{
					ISR::std::cout << "r10 = " << (void*)regs.r10 << '\n';
				}
				else if (subCmd == "r11")
				{
					ISR::std::cout << "r11 = " << (void*)regs.r11 << '\n';
				}
				else if (subCmd == "r12")
				{
					ISR::std::cout << "r12 = " << (void*)regs.r12 << '\n';
				}
				else if (subCmd == "r13")
				{
					ISR::std::cout << "r13 = " << (void*)regs.r13 << '\n';
				}
				else if (subCmd == "r14")
				{
					ISR::std::cout << "r14 = " << (void*)regs.r14 << '\n';
				}
				else if (subCmd == "r15")
				{
					ISR::std::cout << "r15 = " << (void*)regs.r15 << '\n';
				}
				else if (subCmd == "rip")
				{
					ISR::std::cout << "rip = " << (void*)regs.rip << '\n';
				}
				else if (subCmd == "cr3")
				{
					ISR::std::cout << "cr3 = " << (void*)regs.cr3 << '\n';
				}
				else if (subCmd == "regs")
				{
					ISR::std::cout
						<< "\nrax=" << (void *)regs.rax << " rbx=" << (void *)regs.rbx << " rcx=" << (void *)regs.rcx
						<< "\nrdx=" << (void *)regs.rdx << " rdi=" << (void *)regs.rdi << " rsi=" << (void *)regs.rsi
						<< "\nrsp=" << (void *)regs.rsp << " rbp=" << (void *)regs.rbp << " r8 =" << (void *)regs.r8
						<< "\nr9 =" << (void *)regs.r9  << " r10=" << (void *)regs.r10 << " r11=" << (void *)regs.r11
						<< "\nr12=" << (void *)regs.r12 << " r13=" << (void *)regs.r13 << " r14=" << (void *)regs.r14
						<< "\nr15=" << (void *)regs.r15 << '\n';
				}
				else if (subCmd == "c")
				{
					exit_handler = true;
				}
				else
				{
					ull colon = subCmd.firstOf(':');
					ull dash = subCmd.firstOf('-');

					if (dash != std::string::npos)
					{
						// interpret as lower and upper limit
						std::string lowerStr, upperStr;
						upperStr = subCmd.data() + dash + 1;
						lowerStr.assign(subCmd.data(), dash);

						ull lower, upper;

						bool lowerValid = interpretHexStr(lowerStr, lower),
							upperValid = interpretHexStr(upperStr, upper);

						if (!lowerValid)
						{
							ISR::std::cout << "Cannot interpret\"" << lowerStr.data() << "\"\n";
						}
						else if (!upperValid)
						{
							ISR::std::cout << "Cannot interpret\"" << upperStr.data() << "\"\n";
						}
						else
						{
							ull len = upper - lower + 1;
							isr_DisplayMemoryBlock((byte*)lower, len);
						}
					}
					else if (colon != std::string::npos)
					{
						// interpret as base and length
						std::string lowerStr, lenStr;
						lenStr = subCmd.data() + colon + 1;
						lowerStr.assign(subCmd.data(), colon);

						ull lower, len;

						bool lowerValid = interpretHexStr(lowerStr, lower),
							lenValid = interpretHexStr(lenStr, len);

						if (!lowerValid)
						{
							ISR::std::cout << "Cannot interpret\"" << lowerStr.data() << "\"\n";
						}
						else if (!lenValid)
						{
							ISR::std::cout << "Cannot interpret\"" << lenStr.data() << "\"\n";
						}
						else
						{
							isr_DisplayMemoryBlock((byte*)lower, len);
						}
					}
				}
			}
		}

		// disable interrupts and enable scheduling again
		disableInterrupts();
		Scheduler::enable();
		ISR::std::cout << "Continuing...\n";
	}
}