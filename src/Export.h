#pragma once
#include "Expected.h"

#include <string>

namespace renderdesc { class Pipeline; }

class IAutomation;
class Resources;

struct ExportSettings {
	int width = 1920, height = 1080;
	std::string c_source = "export.c";
	std::string shader_path = "";
	std::string shader_suffix = "";
	//bool shader_concat = false;
};

Expected<void, std::string> exportC(Resources &res, const ExportSettings &settings, const renderdesc::Pipeline &p, const IAutomation &automation);
