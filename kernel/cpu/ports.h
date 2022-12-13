#pragma once
#include "../utils/types.h"

extern "C" byte inb(word port);
extern "C" word inw(word port);
extern "C" dword indw(word port);
extern "C" void outb(word port, byte value);
extern "C" void outw(word port, word value);
extern "C" void outdw(word port, dword value);

void io_wait();