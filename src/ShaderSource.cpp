#include "ShaderSource.h"

static bool startsWithAtPos(const std::string_view& str, size_t pos, const std::string_view& substr) {
	if (pos >= str.size())
		return false;

	if (str.size() - pos < substr.size())
		return false;

	return 0 == str.substr(pos, substr.size()).compare(substr);
}

ShaderSource::ShaderSource(std::string&& source, UniformsMap&& uniforms)
	: source_(std::move(source))
	, uniforms_(std::move(uniforms))
{
}

ShaderSource::ShaderSource(ShaderSource&& src)
	: source_(std::move(src.source_))
	, uniforms_(std::move(src.uniforms_))
{
}

ShaderSource::Result ShaderSource::create(const std::string_view& raw_source) {
	std::string source;
	UniformsMap uniforms;

	std::string_view::size_type chunk_begin = 0;
	for (;;) {
		const std::string_view::size_type chunk_end = raw_source.find("$(", chunk_begin);
		const bool found = chunk_end != std::string_view::npos;

		source.append(raw_source.cbegin() + chunk_begin,
			found ? raw_source.cbegin() + chunk_end : raw_source.cend());

		if (!found)
			break;

		size_t cur = chunk_end + 2;
		UniformType type;
		if (startsWithAtPos(raw_source, cur, "float")) {
			type = UniformType::Float;
			cur += 5;
		} else if (startsWithAtPos(raw_source, cur, "vec2")) {
			type = UniformType::Vec2;
			cur += 4;
		} else if (startsWithAtPos(raw_source, cur, "vec3")) {
			type = UniformType::Vec3;
			cur += 4;
		} else if (startsWithAtPos(raw_source, cur, "vec4")) {
			type = UniformType::Vec4;
			cur += 4;
		} else
			return std::string("Unexpected type at ") + std::string(raw_source.substr(chunk_end, 16));

		while (cur < raw_source.size() && isspace(raw_source[cur])) ++cur;
		if (cur >= raw_source.size())
			return std::string("Unexpected end after ") + std::string(raw_source.substr(chunk_end, 16));

		const size_t name_begin = cur;
		char c;
		while (cur < raw_source.size()) {
			c = raw_source[cur];
			if (isspace(c) || c == ')')
				break;

			if (!isalnum(c) && c != '_')
				return std::string("Invalid character in ") + std::string(raw_source.substr(chunk_end, 16));

			++cur;
		}

		if (cur >= raw_source.size())
			return std::string("Unexpected end after ") + std::string(raw_source.substr(chunk_end, 16));

		const size_t name_end = cur;

		while (cur < raw_source.size() && isspace(raw_source[cur])) ++cur;

		if (cur >= raw_source.size() || c != ')')
			return std::string("Unexpected end after ") + std::string(raw_source.substr(chunk_end, 16));

		chunk_begin = cur + 1;

		const std::string name = std::string(raw_source.substr(name_begin, name_end - name_begin));
		const UniformsMap::const_iterator it = uniforms.find(name);
		if (it != uniforms.end()) {
			if (it->second.name != name || it->second.type != type)
				return std::string("Type mismatch for variable ") + name + std::string(" at ") + std::string(raw_source.substr(chunk_end, 16));
		} else {
			uniforms[name] = UniformDeclaration{type, name};
		}

		source += name;
	} // for (;;) raw_source

	return Result(Shared(new ShaderSource(std::move(source), std::move(uniforms))));
}
