#pragma once
#include "Export.h"
#include "Expected.h"
#include <string>

struct ProjectSettings {
	struct {
		std::string config_filename;
	} video;

	struct {
		int samplerate = 0;
		int channels = 0;
		int samples = 0;
		int samples_per_row = 0;
		float *data = nullptr;
	} audio;

	struct Automation {
		enum class Type { None, Basic, Rocket } type = Type::None;
		std::string filename;
	} automation;

	ExportSettings exports;

	static Expected<ProjectSettings, std::string> readFromFile(const char *filename);
};
