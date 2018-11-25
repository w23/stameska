#include "VideoEngine.h"

#include "PolledResource.h"
#include "PolledFile.h"
#include "Program.h"
#include "Timeline.h"

#include <vector>

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

class PolledShaderSource : public PolledResource {
public:
	PolledShaderSource(const std::shared_ptr<PolledFile>& file)
		: file_(file)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq) && file_.poll(poll_seq)) {
			try {
				source_ = shader::Source::load(file_->string());
				return endUpdate();
			}	catch (const std::runtime_error& e) {
				MSG("Error updating shader source: %s", e.what());
			}
		}

		return false;
	}

	const shader::Source& source() const { return source_; }

private:
	PollMux<PolledFile> file_;
	shader::Source source_;
};

class PolledShaderSources : public PolledResource {
public:
	PolledShaderSources(int version, const std::vector<std::shared_ptr<PolledShaderSource>>& polled_sources)
		: version_(version)
		// .... oh god
		, polled_sources_([&polled_sources]() {
		std::vector<PollMux<PolledShaderSource>> ps;
		for (const auto& it : polled_sources) {
			ps.emplace_back(PollMux<PolledShaderSource>(it));
		}
		return ps;}())
	{
	}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq)) {
			try {
				bool need_update = false;
				for (auto &it : polled_sources_)
					need_update |= it.poll(poll_seq);

				if (!need_update)
					return false;

				sources_ = shader::Sources::load(version_,
					MuxShaderConstIterator(polled_sources_.cbegin()),
					MuxShaderConstIterator(polled_sources_.cend()));
				return endUpdate();

			}	catch (const std::runtime_error& e) {
				MSG("Error updating shader source: %s", e.what());
			}
		}

		return false;
	}

	const shader::Sources& sources() const { return sources_; }
private:
	class MuxShaderConstIterator {
	public:
		typedef std::vector<PollMux<PolledShaderSource>>::const_iterator ParentIter;
		MuxShaderConstIterator(ParentIter it) : it_(it) {}

		const shader::Source *operator->() const { return &(*it_).operator->()->source(); }
		bool operator<(const MuxShaderConstIterator& rhs) const { return it_ < rhs.it_;  }
		MuxShaderConstIterator& operator++() { ++it_;  return *this; }
	private:
		ParentIter it_;
	};

private:
	const int version_;
	std::vector<PollMux<PolledShaderSource>> polled_sources_;
	shader::Sources sources_;
};

class PolledShaderProgram : public PolledResource {
public:
	PolledShaderProgram(const std::shared_ptr<PolledShaderSources>& sources)
		: sources_(sources)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq) && sources_->poll(poll_seq)) {
			try {
				program_ = Program::load(sources_->sources());
				return endUpdate();
			} catch (const std::runtime_error& e) {
				MSG("Error updating program: %s", e.what());
			}
		}

		return false;
	}

	const Program& get() const { return program_; }
	const Program& operator->() const { return program_; }

	const shader::UniformsMap& uniforms() const { return sources_->sources().uniforms(); }

private:
	const std::shared_ptr<PolledShaderSources> sources_;
	Program program_;
};

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

void VideoEngine::draw(int w, int h, float row, Timeline &timeline) {
	const auto& main_program = program_.find("main");

	if (main_program == program_.end())
		return;

	const Program& p = main_program->second->get();
	if (!p.valid())
		return;

	p.use();

	for (const auto &it: main_program->second->uniforms()) {
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

	p.setUniform("R", w, h).setUniform("t", row).compute();
}
