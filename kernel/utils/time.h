#pragma once
#include "types.h"

extern "C" qword clock();

class Time
{
	static void IrqHandler();

public:
	static qword time();

	static void Initialize();
};