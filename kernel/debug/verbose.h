#pragma once
#include "../drivers/screen.h"

#define NO_VERBOSE_LOGGING

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(string) Screen::driver_print(string)
#else
#define VERBOSE_LOG(string)
#endif