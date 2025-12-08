#include "../ports.h"
#include "../cpuid.h"
#include "pic.h"
#include "pit.h"
#include "idt.h"

namespace PIC
{
	constexpr word pic1cmd = 0x20,
				   pic1data = pic1cmd + 1,
				   pic2cmd = 0xa0,
				   pic2data = pic2cmd;

	constexpr byte icw4 = 0x01,
				   init = 0x10,
				   icw4_8086 = 0x01,
				   endOfInterrupt = 0x20;

	void EndOfInterrupt(byte irq_no)
	{
		if (irq_no >= 8)
			outb(pic2cmd, endOfInterrupt);

		outb(pic1cmd, endOfInterrupt);
	}

	void Initialize(byte offset)
	{
		byte mask1 = inb(pic1data),
			 mask2 = inb(pic2data);

		outb(pic1cmd, init | icw4);
		outb(pic2cmd, init | icw4);

		io_wait();
		outb(pic1data, offset);
		outb(pic2data, offset + 8);
		io_wait();

		outb(pic1data, 0b100);
		outb(pic2data, 0b10);
		io_wait();

		outb(pic1data, icw4_8086);
		outb(pic2data, icw4_8086);
		io_wait();

		outb(pic1data, mask1);
		outb(pic2data, mask2);

		PIT::ConfigureChannel(PIT::SelectChannel::channel0, PIT::OperatingMode::rateGenerator, 1000 / ms_per_timeint);
		// PIT::ConfigureChannel(PIT::SelectChannel::channel2, PIT::OperatingMode::squareWaveGenerator, 10000 / ms_per_timeint);
		// outb(0x61, inb(0x61) | 0b11);
	}

	constexpr byte readIRRcmd = 0x0a,
				   readISRcmd = 0x0b;
	word getISR()
	{
		outb(pic1cmd, readISRcmd);
		outb(pic2cmd, readISRcmd);
		return (inb(pic2cmd) << 8) | inb(pic1cmd);
	}

	bool detectApic()
	{
		dword unused, edx;
		cpuid(1, unused, unused, unused, edx);
		return (edx >> 9) & 0x1;
	}
}