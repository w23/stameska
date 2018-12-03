#pragma once

#include "PolledResource.h"
#include "PolledFile.h"
#include "Program.h"
#include "string_view.h"
#include "json.hpp"
#include <memory>
#include <map>

using json = nlohmann::json;
class Timeline;
class PolledShaderSource;
class PolledShaderSources;
class PolledShaderProgram;

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
	PolledShaderProgram(const std::shared_ptr<PolledShaderSources> &fragment)
		: fragment_(fragment)
	{
	}

	PolledShaderProgram(const std::shared_ptr<PolledShaderSources> &vertex, const std::shared_ptr<PolledShaderSources> &fragment)
		: vertex_(vertex)
		, fragment_(fragment)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (!beginUpdate(poll_seq))
			return false;

		bool need_update = fragment_->poll(poll_seq);
		need_update |= vertex_ && vertex_->poll(poll_seq);
		if (!need_update)
			return false;

		try {
			if (!vertex_) {
				program_ = Program::load(fragment_->sources());
				uniforms_ = fragment_->sources().uniforms();
			} else {
				Program::Sources srcs;
				srcs.vertex = &vertex_->sources();
				srcs.fragment = &fragment_->sources();
				Program &&new_program = Program::load(srcs);
				if (!new_program.valid())
					throw std::runtime_error("New program is not valid");
				shader::UniformsMap new_uniforms = vertex_->sources().uniforms();
				appendUniforms(new_uniforms, fragment_->sources().uniforms());
				program_ = std::move(new_program);
				uniforms_ = std::move(new_uniforms);
				MSG("Built program %u", program_.name());
			}
			return endUpdate();
		} catch (const std::runtime_error& e) {
			MSG("Error updating program: %s", e.what());
		}

		return false;
	}

	const Program& get() const { return program_; }
	const Program& operator->() const { return program_; }

	const shader::UniformsMap& uniforms() const { return uniforms_; }

private:
	const std::shared_ptr<PolledShaderSources> vertex_, fragment_;
	shader::UniformsMap uniforms_;
	Program program_;
};

class VideoEngine {
public:
	VideoEngine() {}
	VideoEngine(string_view config);
	~VideoEngine();

	void poll(unsigned int poll_seq);

	void draw(int w, int h, float row, Timeline &timeline);

	std::shared_ptr<PolledShaderProgram> getFragmentProgramWithShaders(int version, const std::string &name,
			const std::vector<std::string> &fragment);
	std::shared_ptr<PolledShaderProgram> getProgramWithShaders(int version, const std::string &name,
			const std::vector<std::string> &vertex, const std::vector<std::string> &fragment);
	void useProgram(PolledShaderProgram& program, int w, int h, float row, Timeline &timeline);

private:
	void readPrograms(const json& j);
	void readShaders(const json& j);

	std::shared_ptr<PolledShaderSources> getPolledShaderForFiles(int version, const std::vector<std::string> &vertex);

private:
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_source_;
	std::map<std::string, std::shared_ptr<PolledShaderSources>> shader_sources_;
	std::map<std::string, std::shared_ptr<PolledShaderProgram>> program_;
};
