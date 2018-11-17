#pragma once

#include <string_view>
#include <string>
#include <map>
#include <variant>
#include <memory>

enum class UniformType {
	Float, Vec2, Vec3, Vec4
};

class ShaderSource {
public:
	typedef std::shared_ptr<ShaderSource> Shared;
	typedef std::variant<std::string, ShaderSource::Shared> Result;
	static Result create(const std::string_view& raw_source);

	struct UniformDeclaration {
		UniformType type;
		std::string name;
	};

	typedef std::map<std::string, UniformDeclaration> UniformsMap;

	const std::string& source() const { return source_; }
	const UniformsMap& uniforms() const { return uniforms_; }

private:
	ShaderSource(const ShaderSource&) = delete;
	ShaderSource(std::string&& source, UniformsMap&& uniforms);
	ShaderSource(ShaderSource&&);

	const std::string source_;
	const UniformsMap uniforms_;
};
