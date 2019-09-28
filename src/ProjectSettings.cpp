#include "ProjectSettings.h"
#include "YamlParser.h"
#include "utils.h"

#include <filesystem>
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
	auto video_config_result = map_video_result.value().get().getString("config_file");
	if (!video_config_result)
		return Unexpected("Cannot read video config: " + video_config_result.error());
	settings.video.config_filename = video_config_result.value();

	auto map_audio_result = config.getMapping("audio");
	if (!map_audio_result)
		return Unexpected(map_audio_result.error());
	const yaml::Mapping &yaudio = map_audio_result.value();
	auto samplerate_result = yaudio.getInt("samplerate");
	if (!samplerate_result)
		return Unexpected("Error reading samplerate: " + samplerate_result.error());
	settings.audio.samplerate = samplerate_result.value();
	settings.audio.channels = yaudio.getInt("channels", 2);

	const int duration_sec = yaudio.getInt("duration_sec", 120);
	const int bpm = yaudio.getInt("bpm", 60);
	const int rows_per_beat = yaudio.getInt("rows_per_beat", 8);
	const std::string raw_file = yaudio.getString("raw_file", "");

	settings.audio.samples_per_row = settings.audio.samplerate * 60 / (bpm * rows_per_beat);
	settings.audio.samples = 0;

	if (!raw_file.empty()) {
		FILE *f = fopen((std::filesystem::path(filename).remove_filename()/raw_file).string().c_str(), "rb");
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

	if (auto automation = config.getMapping("automation")) {
		const yaml::Mapping &yautomation = automation.value();
		auto type = yautomation.getString("type");
		if (!type)
			return Unexpected("Cannot read automation: " + type.error());
		const std::string &ytype = type.value();
		if (ytype == "Rocket")
			settings.automation.type = Automation::Type::Rocket;
		else if (ytype == "Basic") {
			settings.automation.type = Automation::Type::Basic;
			auto filename = yautomation.getString("file");
			if (!filename)
				return Unexpected("Cannot read basic automation file: " + filename.error());
			settings.automation.filename = filename.value();
		} else if (ytype == "None") {
		} else
			return Unexpected("Unexpected automation type " + ytype);
	}

	if (auto map_export = config.getMapping("export")) {
		const yaml::Mapping &exports = map_export.value();
	#define READ_VALUE(name, getType) \
		if (auto name = exports.getType(#name)) \
			settings.exports.name = name.value(); \
		else \
			MSG("Cannot read export." #name ": %s. Continuing with default", name.error().c_str())

		READ_VALUE(width, getInt);
		READ_VALUE(height, getInt);
		READ_VALUE(c_source, getString);
		READ_VALUE(shader_path, getString);
		READ_VALUE(shader_suffix, getString);
		//READ_VALUE(shader_concat, getInt);
	} else {
		MSG("Cannot read export section: %s. Continuing with defaults", map_export.error().c_str());
	}

	return settings;
}
