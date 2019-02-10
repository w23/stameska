#pragma once
#include "Expected.h"
#include <string>
#include <stddef.h>

#ifndef _WIN32
#define PRINTF_ATTR(a, b) __attribute__ ((format (printf, a, b)))
#else
#define PRINTF_ATTR(a, b)
#endif

std::string format(const char* str, ...) PRINTF_ATTR(1, 2);

Expected<long int, std::string> intFromString(const std::string &s);

