#pragma once
#include "../drivers/screen.h"

// #define VERBOSE_LOGGING

#ifdef VERBOSE_LOGGING
#define VERBOSE_LOG(string) Screen::driver_print(string)
#define VERBOSE_BEGIN Screen::driver_clear()
#define SCREENDRIVER_PRINT_REDIRECT if (buffer == nullptr) return driver_print_uninitialized(msg)
#define SCREENDRIVER_CLEAR_REDIRECT if (buffer == nullptr) return driver_clear_uninitialized()
#define SCREENDRIVER_INITIALIZE_REDIRECT return Initialize_vervose()
#else
#define VERBOSE_LOG(string)
#define VERBOSE_BEGIN
#define SCREENDRIVER_PRINT_REDIRECT
#define SCREENDRIVER_CLEAR_REDIRECT
#define SCREENDRIVER_INITIALIZE_REDIRECT
#endif