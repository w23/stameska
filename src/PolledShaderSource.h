#pragma once

#include "PolledResource.h"
#include "PolledFile.h"
#include "ShaderSource.h"
#include <memory>

class Resources;

class PolledShaderSource;
typedef std::shared_ptr<PolledShaderSource> shader_ptr;

class PolledShaderSource : public PolledResource {
public:
	PolledShaderSource(Resources &resources, const std::shared_ptr<PolledFile>& file);

	bool poll(unsigned int poll_seq);

	const shader::Source &flatSource() const { return flat_source_; }

private:
	Resources &resources_;
	PollMux<PolledFile> file_;

	struct Chunk {
		enum class Type {
			String, Include, Uniform
		} type;

		std::string string;
		PollMux<PolledShaderSource> include;

		Chunk(std::string&& string) : type(Type::String), string(std::move(string)) {}
		Chunk(const std::string& string) : type(Type::String), string(string) {}
		Chunk(const shader_ptr &include) : type(Type::Include), include(include) {}

		struct Uniform {};
		Chunk(Uniform, const std::string& string) : type(Type::Uniform), string(string) {}
	};

	std::vector<Chunk> chunks_;
	shader::UniformsMap uniforms_;
	shader::Source flat_source_;
};
