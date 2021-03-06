#pragma once
#include "Expected.h"
#include <string>
#include <stddef.h>

#ifndef _WIN32
#define PRINTF_ATTR(a, b) __attribute__ ((format (printf, a, b)))
#else
#define PRINTF_ATTR(a, b)
#endif

#define PRISV(sv) static_cast<int>(sv.size()), sv.data()

std::string format(const char* str, ...) PRINTF_ATTR(1, 2);

Expected<long int, std::string> intFromString(const std::string &s);
Expected<float, std::string> floatFromString(const std::string &s);

