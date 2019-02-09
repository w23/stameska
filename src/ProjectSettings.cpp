#include "ProjectSettings.h"
#include "YamlParser.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

Expected<ProjectSettings, std::string> ProjectSettings::readFromFile(const char *filename) {
	MSG("Reading project settings from file %s", filename);

	const auto top_result = yaml::parse(filename);
	if (!top_result)
		return Unexpected(top_result.error());

	const yaml::Value &top = top_result.value();
	const auto config_result = top.getMapping();
	if (!config_result)
		return Unexpected<std::string>("Top config entity is not an ojbect");
	const yaml::Mapping &config = config_result.value();

	ProjectSettings settings;
	auto map_video_result = config.getMapping("video");
	if (!map_video_result)
		return Unexpected(map_video_result.error());
	settings.video.config_filename = map_video_result.value().get().getString("config_file");

	auto map_audio_result = config.getMapping("audio");
	if (!map_audio_result)
		return Unexpected(map_audio_result.error());
	const yaml::Mapping &yaudio = map_audio_result.value();
	settings.audio.samplerate = yaudio.getInt("samplerate");
	settings.audio.channels = yaudio.getInt("channels", 2);

	const int duration_sec = yaudio.getInt("duration_sec", 120);
	const int bpm = yaudio.getInt("bpm", 60);
	const int rows_per_beat = yaudio.getInt("rows_per_beat", 8);
	const std::string raw_file = yaudio.getString("raw_file", "");

	settings.audio.samples_per_row = settings.audio.samplerate * 60 / (bpm * rows_per_beat);
	settings.audio.samples = 0;

	if (!raw_file.empty()) {
		FILE *f = fopen(raw_file.c_str(), "rb");
		if (f) {
			fseek(f, 0L, SEEK_END);
			settings.audio.samples = ftell(f) / (sizeof(float) * settings.audio.channels);
			fseek(f, 0L, SEEK_SET);
			settings.audio.data = new float[settings.audio.samples * 2];
			const int samples_read = fread(settings.audio.data, sizeof(float) * settings.audio.channels, settings.audio.samples, f);
			if (settings.audio.samples != samples_read || !settings.audio.samples) {
				delete[] settings.audio.data;
				settings.audio.data = nullptr;
			}
			fclose(f);
		}
	}

	if (!settings.audio.data) {
		MSG("No raw audio available, continuing in silence");
		settings.audio.samples = settings.audio.samplerate * duration_sec;
	}

	return settings;
}
