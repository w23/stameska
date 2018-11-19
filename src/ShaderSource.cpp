#include "ShaderSource.h"

static bool startsWithAtPos(const std::string_view& str, size_t pos, const std::string_view& substr) {
	if (pos >= str.size())
		return false;

	if (str.size() - pos < substr.size())
		return false;

	return 0 == str.substr(pos, substr.size()).compare(substr);
}

static const char *uniformName(shader::UniformType type) {
	switch(type) {
		case shader::UniformType::Float: return "float";
		case shader::UniformType::Vec2: return "vec2";
		case shader::UniformType::Vec3: return "vec3";
		case shader::UniformType::Vec4: return "vec4";
		default: return "INVALID";
	}
}

namespace shader {

Source::Source(std::string&& source, UniformsMap&& uniforms)
	: source_(std::move(source))
	, uniforms_(std::move(uniforms))
{
}

Source Source::load(const std::string_view& raw_source) {
	std::string source;
	UniformsMap uniforms;

	// Read the entire source looking for "$(type name)" pattern. These inline
	// definitions will become uniforms and will be replaced by just "name".
	// Everything in-between these definitions are chunks that will be copied
	// as-is.
	std::string_view::size_type chunk_begin = 0;
	for (;;) {
		const std::string_view::size_type chunk_end = raw_source.find("$(", chunk_begin);
		const bool found = chunk_end != std::string_view::npos;

		// Copy chunk regardless of whether a new inline uniform was found
		source.append(raw_source.cbegin() + chunk_begin,
			found ? raw_source.cbegin() + chunk_end : raw_source.cend());

		if (!found)
			break;

		const std::string chunk_desc(raw_source.substr(chunk_end, 16));

		// Read type
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
			throw std::runtime_error(format("Unexpected type at %s",  chunk_desc.c_str()));

		// Skip spaces
		while (cur < raw_source.size() && isspace(raw_source[cur])) ++cur;
		if (cur >= raw_source.size())
			throw std::runtime_error(format("Unexpected end at %s",  chunk_desc.c_str()));

		// Read name
		const size_t name_begin = cur;
		char c;
		while (cur < raw_source.size()) {
			c = raw_source[cur];
			if (isspace(c) || c == ')')
				break;

			if (!isalnum(c) && c != '_')
				throw std::runtime_error(format("Invalid char %c at %s", c, chunk_desc.c_str()));

			++cur;
		}

		if (cur >= raw_source.size())
			throw std::runtime_error(format("Unexpected end after %s", chunk_desc.c_str()));

		const size_t name_end = cur;

		// Skip spaces
		while (cur < raw_source.size() && isspace(raw_source[cur])) ++cur;

		// Find terminator
		if (cur >= raw_source.size() || c != ')')
			throw std::runtime_error(format("Unexpected end after %s", chunk_desc.c_str()));

		chunk_begin = cur + 1;

		// Store new inline uniform in table
		const std::string name = std::string(raw_source.substr(name_begin, name_end - name_begin));
		const UniformsMap::const_iterator it = uniforms.find(name);
		if (it != uniforms.end()) {
			if (it->second.name != name || it->second.type != type)
				throw std::runtime_error(format("Type mismatch for variable %s at %s", name.c_str(), chunk_desc.c_str()));
		} else {
			uniforms[name] = UniformDeclaration{type, name};
		}

		source += std::move(name);
	} // for (;;) raw_source

	return Source(std::move(source), std::move(uniforms));
}

Sources Sources::load(const std::vector<Source>& sources) {
	UniformsMap uniforms;

	// Merge all uniforms
	for (const auto& src: sources) {
		for (const auto& uni: src.uniforms()) {
			const UniformsMap::const_iterator it = uniforms.find(uni.first);
			if (it != uniforms.end()) {
				if (it->second.type != uni.second.type)
					throw std::runtime_error(format("Type mismatch for uniform %s: %s != %s",
							uni.first.c_str(), uniformName(uni.second.type), uniformName(it->second.type)));
			} else
				uniforms[uni.first] = uni.second;
		}
	}

	// Create unifor declaration header
	std::string source;
	for (const auto& uni: uniforms)
		source += format("uniform %s %s;\n", uniformName(uni.second.type), uni.second.name.c_str());

	MSG("%s", source.c_str());

	for( const auto& src: sources) {
		source += src.source();
		source += "\n";
	}

	return Sources(std::move(source), std::move(uniforms));
}

Sources::Sources(std::string&& source, UniformsMap&& uniforms)
	: source_(std::move(source))
	, uniforms_(std::move(uniforms))
{
}

} // namespace shader
