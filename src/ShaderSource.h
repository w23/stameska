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

class Sources {
public:
	Sources() {}
	~Sources() {}
	Sources(Sources&&) = default;

	static Sources load(const std::vector<Source>& source);

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
