#pragma once
#include <types.h>

namespace PIC
{
	void EndOfInterrupt(byte irq_no);
	void Initialize(byte offset);
	bool detectApic();
	word getISR();
};