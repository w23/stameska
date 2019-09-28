#pragma once

// Pull in __GLIBCXX__ definition
#include <cstdint>

#if defined(__GLIBCXX__) && __GLIBCXX__ >= 20180502
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
