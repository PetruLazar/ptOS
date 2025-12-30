#pragma once
#include "../cpu/interrupt/idt.h"
#include "../cpu/interrupt/irq.h"

namespace Debug
{
    // single step
    // breakpoints

    void DebugExceptionHandler(registers_t &regs, qword int_no);
}