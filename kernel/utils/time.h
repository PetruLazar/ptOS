#pragma once
#include "../../libc/types.h"

extern "C" qword clock();

class Time
{
	static void IrqHandler();

public:
	static qword time();

	static void Initialize();
};