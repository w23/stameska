#pragma once
#include "Expected.h"

namespace renderdesc { class Pipeline; }

Expected<void, std::string> exportC(const renderdesc::Pipeline &p, int w, int h, const char *filename);
