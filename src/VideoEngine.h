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

class VideoEngine {
public:
	VideoEngine() {}
	VideoEngine(string_view config);
	~VideoEngine();

	void poll(unsigned int poll_seq);

	void draw(int w, int h, float row, Timeline &timeline);

	std::shared_ptr<PolledShaderProgram> getFragmentProgramWithShaders(int version, const std::string &name, const std::vector<std::string> &shaders);
	void useProgram(PolledShaderProgram& program, int w, int h, float row, Timeline &timeline);

private:
	void readPrograms(const json& j);
	void readShaders(const json& j);

private:
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_source_;
	std::map<std::string, std::shared_ptr<PolledShaderSources>> shader_sources_;
	std::map<std::string, std::shared_ptr<PolledShaderProgram>> program_;
};
