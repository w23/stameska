#pragma once

#include "Expected.h"

#include <string>
#include <map>
#include <vector>
#include <string_view>

namespace shader {

enum class UniformType {
	Float, Vec2, Vec3, Vec4
};

const char *uniformName(shader::UniformType type);

struct UniformDeclaration {
	UniformType type;
	std::string name;
};

typedef std::map<std::string, UniformDeclaration> UniformsMap;

Expected<void, std::string> appendUniforms(UniformsMap &uniforms, const UniformsMap &append);

class Source {
public:
	Source() {}
	~Source() {}
	Source(Source&&) = default;

	static Expected<Source, std::string> load(std::string_view raw_source);

	struct Chunk {
		enum class Type {
			String,
			Include,
		} type;

		std::string value;

		Chunk(Type type, std::string_view value) : type(type), value(value) {}
		Chunk(Type type, std::string&& value) : type(type), value(std::move(value)) {}
	};

	const std::vector<Chunk> &chunks() const { return chunks_; }
	const UniformsMap& uniforms() const { return uniforms_; }
	int version() const { return version_; }

	Source& operator=(Source&& other) = default;

private:
	Source(const Source&) = delete;
	Source(int version, std::vector<Chunk>&& chunks, UniformsMap&& uniforms);

	int version_ = 0;
	std::vector<Chunk> chunks_;
	UniformsMap uniforms_;
};

}
