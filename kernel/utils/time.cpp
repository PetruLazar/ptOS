#include "time.h"
#include "../cpu/idt.h"

qword irqCount = 0;

void Time::IrqHandler()
{
	irqCount++;
}

void Time::Initialize()
{
	IDT::registerIrqHandler(0, Time::IrqHandler);
}

qword Time::time()
{
	return irqCount;
}