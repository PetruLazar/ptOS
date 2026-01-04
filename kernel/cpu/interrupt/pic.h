#pragma once
#include <types.h>

namespace PIC
{
	void EndOfInterrupt(byte irq_no);
	void Initialize(byte offset);
	void Disable();
	word getISR();
};