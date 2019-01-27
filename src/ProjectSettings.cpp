#include "ProjectSettings.h"
#include "YamlParser.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

void ProjectSettings::readFromFile(const char *filename) {
	MSG("Reading project settings from file %s", filename);

	const yaml::Value top = yaml::parse(filename);
	const yaml::Mapping &config = top.getMapping();

	video.config_filename = config.getMapping("video").getString("config_file");

	const yaml::Mapping &yaudio = config.getMapping("audio");
	audio.samplerate = yaudio.getInt("samplerate");
	audio.channels = yaudio.getInt("channels", 2);

	const int duration_sec = yaudio.getInt("duration_sec", 120);
	const int bpm = yaudio.getInt("bpm", 60);
	const int rows_per_beat = yaudio.getInt("rows_per_beat", 8);
	const std::string raw_file = yaudio.getString("raw_file", "");

	audio.samples_per_row = audio.samplerate * 60 / (bpm * rows_per_beat);
	audio.samples = 0;

	if (!raw_file.empty()) {
		FILE *f = fopen(raw_file.c_str(), "rb");
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
	}

	if (!audio.data) {
		MSG("No raw audio available, continuing in silence");
		audio.samples = audio.samplerate * duration_sec;
	}
}
