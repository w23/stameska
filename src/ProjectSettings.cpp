#include "ProjectSettings.h"
#include "utils.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

static json readJsonFromFile(const char *filename) {
	try {
		std::ifstream i(filename, std::ifstream::in);
		json j;
		i >> j;
		return j;
	} catch (const std::exception& e) {
		MSG("Error reading file %s: %s", filename, e.what());
		return json();
	}
}

void ProjectSettings::readFromFile(const char *filename) {
	MSG("Reading project settings from file %s", filename);
	json j = readJsonFromFile(filename);
	if (j.is_null())
		j = json::object();

	const json& jvideo = j.at("video");
	video.config_filename = jvideo.at("config_file");

	const json& jaudio = j.at("audio");

	audio.samplerate = jaudio.at("samplerate");
	audio.channels = jaudio.at("channels");
	audio.samples_per_row = audio.samplerate * 60 / (jaudio.at("bpm").get<int>() * jaudio.at("rows_per_beat").get<int>());
	audio.samples = 0;

	FILE *f = nullptr;
	const std::string& raw_file = jaudio.value("raw_file", "");
	f = fopen(raw_file.c_str(), "rb");
	if (f) {
		fseek(f, 0L, SEEK_END);
		audio.samples = ftell(f) / (sizeof(float) * audio.channels);
		fseek(f, 0L, SEEK_SET);
		audio.data = new float[audio.samples * 2];
		const int samples_read = fread(audio.data, sizeof(float) * audio.channels, audio.samples, f);
		if (audio.samples != samples_read || !audio.samples) {
			delete[] audio.data;
			audio.data = nullptr;
		}
		fclose(f);
	}

	if (!audio.data) {
		MSG("No raw audio available, continuing in silence");
		audio.samples = audio.samplerate * j.value("duration_sec", 120);
	}
}
