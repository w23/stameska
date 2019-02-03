#pragma once

namespace renderdesc { class Pipeline; }

void exportC(const renderdesc::Pipeline &p, int w, int h, const char *filename);
