#include "VideoEngine.h"

#include "Timeline.h"

#include <vector>
#include <set>

/* FIXME
const GLuint Framebuffer::draw_buffers_[4] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};

#ifdef DO_NOISE
#define NOISE_SIZE 1024
uint32_t noise_buffer[NOISE_SIZE * NOISE_SIZE];
static void initNoise(Texture &noise) {
	pcg32_random_t rand = PCG32_INITIALIZER;
	for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i) {
		noise_buffer[i] = pcg32_random_r(&rand);
	}

	noise.upload(NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise_buffer);
}
#endif
*/


VideoEngine::VideoEngine(string_view config) {
	const json root = json::parse(config.data(), config.data() + config.size());

	readShaders(root.at("shaders"));
	readPrograms(root.at("programs"));
}

VideoEngine::~VideoEngine() {}

void VideoEngine::readShaders(const json& j) {
	for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
		const std::string &name = it.key();
		const json &jshader = it.value();
		const int version = jshader.value("version", 450);
		const json &jsources = jshader.at("sources");

		std::vector<std::shared_ptr<PolledShaderSource>> sources;
		for (const auto &jsrc: jsources) {
			const std::string& source_file = jsrc.get<std::string>();
			std::shared_ptr<PolledShaderSource> source;
			const auto cached = shader_source_.find(name);
			if (cached != shader_source_.end()) {
				source = cached->second;
			} else {
				MSG("Adding shader file %s", source_file.c_str());
				source.reset(new PolledShaderSource(std::shared_ptr<PolledFile>(new PolledFile(source_file))));
				shader_source_[source_file] = source;
			}
			sources.emplace_back(std::move(source));
		}

		MSG("Adding shader %s", name.c_str());
		shader_sources_[name]  = std::shared_ptr<PolledShaderSources>(new PolledShaderSources(version, std::move(sources)));
	}
}

void VideoEngine::readPrograms(const json& j) {
	for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
		const std::string& name = it.key();
		const json& jprog = it.value();

		const std::string& shader_name = jprog.at("fragment");
		MSG("Adding program %s with shader %s", name.c_str(), shader_name.c_str());
		program_[name] = std::unique_ptr<PolledShaderProgram>(new PolledShaderProgram(shader_sources_[shader_name]));
	}
}

void VideoEngine::poll(unsigned int poll_seq) {
	for (auto &p : program_)
		p.second->poll(poll_seq);
}

static const std::set<std::string> internal_uniforms = {"R", "t"};

void VideoEngine::draw(int w, int h, float row, Timeline &timeline) {
	const auto& main_program = program_.find("main");

	if (main_program == program_.end())
		return;

	useProgram(*main_program->second.get(), w, h, row, timeline);
	const Program& p = main_program->second->get();
	p.compute();
}

std::shared_ptr<PolledShaderProgram> VideoEngine::getFragmentProgramWithShaders(int version, const std::string &name, const std::vector<std::string> &shaders) {
	std::vector<std::shared_ptr<PolledShaderSource>> sources;
	for (const auto &s: shaders) {
		std::shared_ptr<PolledShaderSource> source;
		const auto cached = shader_source_.find(s);
		if (cached != shader_source_.end()) {
			source = cached->second;
		} else {
			MSG("Adding shader file %s", s.c_str());
			source.reset(new PolledShaderSource(std::shared_ptr<PolledFile>(new PolledFile(s))));
			shader_source_[s] = source;
		}
		sources.emplace_back(std::move(source));
	}

	std::shared_ptr<PolledShaderSources> polled_sources;
	const auto cached_sources = shader_sources_.find(name);
	if (cached_sources != shader_sources_.end()) {
		polled_sources = cached_sources->second;
	} else {
		MSG("Adding shader %s", name.c_str());
		polled_sources.reset(new PolledShaderSources(version, std::move(sources)));
		shader_sources_[name]  = polled_sources;
	}

	std::shared_ptr<PolledShaderProgram> polled_program;
	const auto cached_program = program_.find(name);
	if (cached_program != program_.end()) {
		polled_program = cached_program->second;
	} else {
		MSG("Adding program %s", name.c_str());
		polled_program.reset(new PolledShaderProgram(polled_sources));
		program_[name] = polled_program;
	}

	return polled_program;
}

void VideoEngine::useProgram(PolledShaderProgram& program, int w, int h, float row, Timeline &timeline) {
	const Program& p = program.get();
	if (!p.valid())
		return;

	p.use();

	for (const auto &it: program.uniforms()) {
		if (internal_uniforms.find(it.first) != internal_uniforms.end())
			continue;

		const Value v = timeline.getValue(it.first, static_cast<int>(it.second.type) + 1);
		switch (it.second.type) {
			case shader::UniformType::Float:
				p.setUniform(it.first.c_str(), v.x);
				break;
			case shader::UniformType::Vec2:
				p.setUniform(it.first.c_str(), v.x, v.y);
				break;
			case shader::UniformType::Vec3:
				p.setUniform(it.first.c_str(), v.x, v.y, v.z);
				break;
			case shader::UniformType::Vec4:
				p.setUniform(it.first.c_str(), v.x, v.y, v.z, v.w);
				break;
		}
	}

	p.setUniform("R", w, h).setUniform("t", row);
}
