#pragma once
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

	static Expected<ProjectSettings, std::string> readFromFile(const char *filename);
};
