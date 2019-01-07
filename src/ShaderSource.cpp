#include "ShaderSource.h"
#include <regex>

namespace shader {

const char *uniformName(shader::UniformType type) {
	switch(type) {
		case shader::UniformType::Float: return "float";
		case shader::UniformType::Vec2: return "vec2";
		case shader::UniformType::Vec3: return "vec3";
		case shader::UniformType::Vec4: return "vec4";
		default: return "INVALID";
	}
}

Source::Source(std::vector<Chunk>&& chunks, UniformsMap&& uniforms)
	: chunks_(std::move(chunks))
	, uniforms_(std::move(uniforms))
{
}

static const std::regex reg_inluniforms("#(\\w+)\\s+(\\w+)");
static const std::regex reg_includes("#include \"(.*)\"");

using svregex_iterator = std::regex_iterator<string_view::const_iterator>;
using svmatch = std::match_results<string_view::const_iterator>;

static std::string extractInlineUniforms(string_view chunk, UniformsMap &uniforms) {
	std::string ret;

	const auto uniforms_begin = svregex_iterator(chunk.begin(), chunk.end(), reg_inluniforms);
	const auto uniforms_end = svregex_iterator();
	auto uniform_chunk_begin = chunk.begin();
	for (auto uni = uniforms_begin; uni != uniforms_end; ++uni) {
		const svmatch &um = *uni;
		const auto uniform_chunk_end = chunk.begin() + um.position(0);
		ret.append(uniform_chunk_begin, uniform_chunk_end);
		uniform_chunk_begin = uniform_chunk_end + um.length(0);

		const string_view found_type = um.str(1);
		UniformType type;
		if (0 == found_type.compare("float")) {
			type = UniformType::Float;
		} else if (0 == found_type.compare("vec2")) {
			type = UniformType::Vec2;
		} else if (0 == found_type.compare("vec3")) {
			type = UniformType::Vec3;
		} else if (0 == found_type.compare("vec4")) {
			type = UniformType::Vec4;
		} else
			throw std::runtime_error(format("Unexpected type at %s",  um.str(0).c_str()));

		// Store new inline uniform in table
		const std::string name = um.str(2);
		const UniformsMap::const_iterator it = uniforms.find(name);
		if (it != uniforms.end()) {
			if (it->second.name != name || it->second.type != type)
				throw std::runtime_error(format("Type mismatch for variable %s at %s", name.c_str(), um.str(0).c_str()));
		} else {
			uniforms[name] = UniformDeclaration{type, name};
		}

		ret.append(name);
	} // for all inline uniforms

	const auto uniform_chunk_end = chunk.end();
	ret.append(uniform_chunk_begin, uniform_chunk_end);

	return ret;
}

Source Source::load(string_view raw_source) {
	std::string chunk_source;
	UniformsMap uniforms;
	std::vector<Chunk> chunks;

	// Read the entire source looking for preprocessor patterns:
	// - '#type name' for inline uniform definitions. These will be replaced by
	//   just "name" and stored in uniforms map.
	// - '#include "filename"' for external file includes
	// Everything else and in-between will be stored as string chunks as-is.

	const auto includes_begin = svregex_iterator(raw_source.begin(), raw_source.end(), reg_includes);
	const auto includes_end = svregex_iterator();

	auto chunk_begin = raw_source.begin();
	for (auto inc = includes_begin; inc != includes_end; ++inc) {
		const svmatch &m = *inc;
		const auto chunk_end = raw_source.begin() + m.position(0);
		std::string chunk = extractInlineUniforms(string_view(chunk_begin, chunk_end - chunk_begin), uniforms);
		chunk_begin = chunk_end + m.length(0);

		chunks.emplace_back(Chunk::Type::String, std::move(chunk));
		chunks.emplace_back(Chunk::Type::Include, std::move(m.str(1)));
	} // for all #includes

	const auto chunk_end = raw_source.end();
	std::string chunk = extractInlineUniforms(string_view(chunk_begin, chunk_end - chunk_begin), uniforms);

	chunks.emplace_back(Chunk::Type::String, std::move(chunk));

	return Source(std::move(chunks), std::move(uniforms));
}

void appendUniforms(UniformsMap &uniforms, const UniformsMap &append) {
	for (const auto &uni: append) {
		const UniformsMap::const_iterator it = uniforms.find(uni.first);
		if (it != uniforms.end()) {
			if (it->second.type != uni.second.type)
				throw std::runtime_error(format("Type mismatch for uniform %s: %s != %s",
						uni.first.c_str(), uniformName(uni.second.type), uniformName(it->second.type)));
		} else
			uniforms[uni.first] = uni.second;
	}
}

Sources::Sources(std::string&& source, UniformsMap&& uniforms)
	: source_(std::move(source))
	, uniforms_(std::move(uniforms))
{
}

} // namespace shader
