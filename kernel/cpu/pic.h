#pragma once
#include "../../libc/types.h"

class PIC
{
public:
	static void EndOfInterrupt(byte irq_no);
	static void Initialize(byte offset);
	static bool detectApic();
	static word getISR();
};