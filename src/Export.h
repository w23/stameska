#pragma once
#include "Expected.h"
#include <string>

namespace renderdesc { class Pipeline; }

struct ExportSettings {
	int width = 1920, height = 1080;
	std::string c_source = "export.c";
	std::string shader_path = "";
	std::string shader_suffix = "";
	//bool shader_concat = false;
};

Expected<void, std::string> exportC(const ExportSettings& settings, const renderdesc::Pipeline &p);
