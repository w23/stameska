#pragma once
#include "atto/app.h"

#define MSG(...) aAppDebugPrintf(__VA_ARGS__)
#define CRASH(...) do { \
	aAppDebugPrintf("[CRASH] " __VA_ARGS__); \
	aAppTerminate(-1); \
} while (0) \

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
