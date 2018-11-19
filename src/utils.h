#pragma once

#include <string>
#include <stddef.h>

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

template <typename T>
class ConstArrayView {
	const T *ptr_;
	size_t size_;

public:
	ConstArrayView(const T& single) : ptr_(&single), size_(1) {}
	ConstArrayView(const T* ptr, size_t size) : ptr_(ptr), size_(size) {}

	size_t size() const { return size_; }
	const T *ptr() const { return ptr_; }
};

#ifndef TOOL
#define MSG(str) MessageBoxA(NULL, str, "Error", MB_OK)
#else
#include "atto/app.h"
#define MSG(...) aAppDebugPrintf(__VA_ARGS__)
#endif

std::string format(const char* str, ...) __attribute__ ((format (printf, 1, 2)));
