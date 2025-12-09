#pragma once
#include <types.h>
#include "idt.h"

namespace IRQ
{
	constexpr int ms_per_timeint = 10;
	
	typedef void (*IrqHandler)(registers_t &); // pointer to void returning function with a registers_t& arg

	enum class Irq_no
	{
		timer = 0,
		ps2_keyboard = 1,

		ps2_mouse = 12,
		primaryATA = 14,
		secondaryATA = 15
	};

	void Initialize();
	void CleanUp();

	void registerIrqHandler(byte irq_no, IrqHandler handler);
}