#pragma once
#include <types.h>

extern "C" qword clock();

namespace Time
{
	void Initialize();

	qword driver_time();
}