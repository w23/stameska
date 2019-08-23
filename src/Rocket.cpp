#include "Rocket.h"
#include "ShaderSource.h"
#include "format.h"
#include "utils.h"

#include "rocket/lib/sync.h"

#include <math.h>
#include <cassert>

Rocket::Rocket(PauseCallback&& pause, SetRowCallback&& setRow, IsPlayingCallback&& isPlaying)
	: pauseCallback(pause)
	, setRowCallback(setRow)
	, isPlayingCallback(isPlaying)
	, rocket_(sync_create_device("sync"))
{
	if (!rocket_)
		CRASH("Cannot create rocket");

	sync_tcp_connect(rocket_.get(), "localhost", SYNC_DEFAULT_PORT);
}

Rocket::~Rocket() {
	sync_save_tracks(rocket_.get());
}

void Rocket::update(float row) {
	struct sync_cb callbacks = { pause, set_row, is_playing };
	if (sync_update(rocket_.get(), (int)floorf(row), &callbacks, this))
		sync_tcp_connect(rocket_.get(), "localhost", SYNC_DEFAULT_PORT);

	row_ = row;
}

static const char* component_suffix[4] = { ".x", ".y", ".z", ".w" };

Value Rocket::getValue(const std::string& name, int comps) {
	assert(comps > 0);
	assert(comps <= 4);

	Value v = { 0, 0, 0, 0 };
	const auto it = vars_.find(name);
	if (it != vars_.end()) {
		for (int i = 0; i < comps; ++i)
			(&v.x)[i] = sync_get_val(it->second.component[i].track, row_);
		if (it->second.components != comps) {
			if (it->second.components < comps) {
				for (int i = it->second.components; i < comps; ++i) {
					it->second.component[i].name = name + component_suffix[i];
					it->second.component[i].track = sync_get_track(rocket_.get(), it->second.component[i].name.c_str());
				}
			} else if (comps == 1) {
				it->second.component[0].name = name;
				it->second.component[0].track = sync_get_track(rocket_.get(), it->second.component[0].name.c_str());
			}
			it->second.components = comps;
		}

		return v;
	}

	Variable var;
	var.components = comps;
	if (comps > 1) {
		for (int i = 0; i < comps; ++i) {
			var.component[i].name = name + component_suffix[i];
			var.component[i].track = sync_get_track(rocket_.get(), var.component[i].name.c_str());
		}
	} else {
		var.component[0].name = name;
		var.component[0].track = sync_get_track(rocket_.get(), var.component[0].name.c_str());
	}

	vars_[name] = std::move(var);
	return v;
}

void Rocket::save() const {
	sync_save_tracks(rocket_.get());
}

#if 0
static Expected<void, std::string> writeUniformTrackData(FILE *out, const std::string &name) {
	std::string filename = "sync_" + name + ".track";

	const auto in = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename.c_str(), "rb"), &fclose);
	if (!in)
		return Unexpected(format("Cannot open file '%s' for reading", filename.c_str()));

	fseek(in.get(), 0, SEEK_END);
	const size_t size = static_cast<size_t>(ftell(in.get()));
	fseek(in.get(), 0, SEEK_SET);

	std::vector<unsigned char> data;
	data.resize(size);

	const size_t read = fread(data.data(), 1, size, in.get());
	if (size != read)
		return Unexpected(format("Could read only %d of %d bytes from '%s'", (int)read, (int)size, filename.c_str()));

	fprintf(out, "\t{\"%s\", %d, \"", filename.c_str(), (int)size);
	for (const auto &c: data)
		fprintf(out, "\\x%x", c);
	fprintf(out, "\", 0},\n");

	return Expected<void, std::string>();
}
#endif

Expected<IAutomation::ExportResult, std::string> Rocket::writeExport(std::string_view config, const shader::UniformsMap &uniforms) const {
	if (config != "C")
		return Unexpected(format("Rocket doesn't support export config '%.*s'", PRISV(config)));

	(void)uniforms;
	return Unexpected<std::string>("Not implemented");

#if 0
	std::vector<std::string> rocket_tracks;
	std::vector<std::pair<int,shader::UniformType>> rocket_tracks_details;
	std::vector<shader::UniformsMap> program_uniforms;
	int rocket_track_index = 0;
	for (const auto &[name, decl]: uniforms) {
		if (name != "R" && name != "t"
			&& std::find(rocket_tracks.begin(), rocket_tracks.end(), name) == rocket_tracks.end()) {

			rocket_tracks.push_back(name);
			rocket_tracks_details.emplace_back(rocket_track_index, decl.type);

			switch (decl.type) {
				case shader::UniformType::Float:
					rocket_track_index += 1;
					break;
				case shader::UniformType::Vec2:
					rocket_track_index += 2;
					break;
				case shader::UniformType::Vec3:
					rocket_track_index += 3;
					break;
				case shader::UniformType::Vec4:
					rocket_track_index += 4;
					break;
			}
		}
	}

	fprintf(f.get(), "static struct RocketTrack rocket_tracks[%d] = {\n", rocket_track_index);
	for (size_t i = 0; i < rocket_tracks.size(); ++i) {
		const auto &name = rocket_tracks[i];
		const auto type = rocket_tracks_details[i].second;

		switch (type) {
			case shader::UniformType::Float:
#define CALL_CHECK(c) do { \
	auto result = c; \
	if (!result) \
		return Unexpected(result.error()); \
} while (0)
				CALL_CHECK(writeUniformTrackData(f.get(), name));
				break;
			case shader::UniformType::Vec2:
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".x"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".y"));
				break;
			case shader::UniformType::Vec3:
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".x"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".y"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".z"));
				break;
			case shader::UniformType::Vec4:
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".x"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".y"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".z"));
				CALL_CHECK(writeUniformTrackData(f.get(), name + ".w"));
				break;
		}
	}
	fprintf(f.get(), "};\n\n");

	if (rocket_track_index)
		fprintf(f.get(), "static const struct sync_track *tracks[%d];\n", rocket_track_index);

	// fprintf(f.get(), "\nstatic void videoInit() {\n");

	rocket_track_index = 0;
	for (size_t i = 0; i < rocket_tracks.size(); ++i) {
		const auto &name = rocket_tracks[i];
		const auto type = rocket_tracks_details[i].second;

		switch (type) {
			case shader::UniformType::Float:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, name.c_str());
				break;
			case shader::UniformType::Vec2:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				break;
			case shader::UniformType::Vec3:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".z").c_str());
				break;
			case shader::UniformType::Vec4:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".z").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".w").c_str());
				break;
			}
	}

	// uniform loading
	const auto &uniforms = program_uniforms[pi];
	for (const auto &[name, decl]: uniforms) {
		const int track_index = (int)(std::find(rocket_tracks.begin(), rocket_tracks.end(), name) - rocket_tracks.begin());
		if (track_index != (int)rocket_tracks.size()) {
			const auto &index_type = rocket_tracks_details[track_index];
			switch (decl.type) {
				case shader::UniformType::Float:
					fprintf(f.get(), "\tglUniform1f(glGetUniformLocation(current_program, \"%s\"), sync_get_val(tracks[%d], t));\n", name.c_str(), index_type.first);
					break;
				case shader::UniformType::Vec2:
					fprintf(f.get(), "\tglUniform2f(glGetUniformLocation(current_program, \"%s\"), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t));\n", name.c_str(),
							index_type.first,
							index_type.first+1);
					break;
				case shader::UniformType::Vec3:
					fprintf(f.get(), "\tglUniform3f(glGetUniformLocation(current_program, \"%s\"), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t));\n", name.c_str(),
							index_type.first,
							index_type.first+1,
							index_type.first+2);
					break;
				case shader::UniformType::Vec4:
					fprintf(f.get(), "\tglUniform4f(glGetUniformLocation(current_program, \"%s\"), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t), "
							"sync_get_val(tracks[%d], t));\n", name.c_str(),
							index_type.first,
							index_type.first+1,
							index_type.first+2,
							index_type.first+3);
					break;
				}
		}
	}
#endif
}

void Rocket::pause(void *t, int p) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	timeline->pauseCallback(p);
}

void Rocket::set_row(void *t, int r) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	timeline->setRowCallback(r);
}

int Rocket::is_playing(void *t) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	return timeline->isPlayingCallback();
}
