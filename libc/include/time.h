#pragma once
#include "types.h"

namespace Time
{
	qword time();

	inline void sleep(int ms)
	{
		ms /= 10; // transform ms in interrupts
		qword stop = time() + ms;
		while (time() < stop)
			;
	}
};