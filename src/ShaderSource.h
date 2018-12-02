#pragma once

#include "string_view.h"
#include "utils.h"

#include <string>
#include <map>
#include <vector>
#include <memory>

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

class Source {
public:
	Source() {}
	~Source() {}
	Source(Source&&) = default;

	static Source load(string_view raw_source);

	const std::string& source() const { return source_; }
	const UniformsMap& uniforms() const { return uniforms_; }

	Source& operator=(Source&& other) = default;

private:
	Source(const Source&) = delete;
	Source(std::string&& source, UniformsMap&& uniforms);

	std::string source_;
	UniformsMap uniforms_;
};

void appendUniforms(UniformsMap &uniforms, const UniformsMap &append);

class Sources {
public:
	Sources() {}
	~Sources() {}
	Sources(Sources&&) = default;

	template <typename I>
	static Sources load(int version, I begin, I end) {
		UniformsMap uniforms;

		// Merge all uniforms
		for (I src = begin; src < end; ++src)
			appendUniforms(uniforms, src->uniforms());

		// Create unifor declaration header
		std::string source = format("#version %d\n", version);
		for (const auto &uni: uniforms)
			source += format("uniform %s %s;\n", uniformName(uni.second.type), uni.second.name.c_str());

		for (I src = begin; src < end; ++src) {
			source += src->source();
			source += "\n";
		}

		return Sources(std::move(source), std::move(uniforms));
	}

	const std::string& source() const { return source_; }
	const UniformsMap& uniforms() const { return uniforms_; }

	Sources& operator=(Sources&& other) = default;

private:
	Sources(const Sources&) = delete;
	Sources(std::string&& source, UniformsMap&& uniforms);

	std::string source_;
	UniformsMap uniforms_;
};

}
