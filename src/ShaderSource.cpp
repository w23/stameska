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

Source::Source(int version, std::vector<Chunk>&& chunks, UniformsMap&& uniforms)
	: version_(version)
	, chunks_(std::move(chunks))
	, uniforms_(std::move(uniforms))
{
}

static const std::regex reg_preprocessor("#(\\w+)\\s+((\\w+)|(\"[^\"]+\"))");

using svregex_iterator = std::regex_iterator<std::string_view::const_iterator>;
using svmatch = std::match_results<std::string_view::const_iterator>;

Source Source::load(std::string_view raw_source) {
	std::string chunk_source;
	UniformsMap uniforms;
	std::vector<Chunk> chunks;
	int version = 0;

	// Read the entire source looking for preprocessor patterns:
	// - '#type name' for inline uniform definitions. These will be replaced by
	//   just "name" and stored in uniforms map.
	// - '#include "filename"' for external file includes
	// - '#version ver' to specify min version (will be propagated to program)
	// Everything else and in-between will be stored as string chunks as-is.

	const auto includes_begin = svregex_iterator(raw_source.begin(), raw_source.end(), reg_preprocessor);
	const auto includes_end = svregex_iterator();

	std::string current_chunk;
	auto subchunk_begin = raw_source.begin();
	for (auto inc = includes_begin; inc != includes_end; ++inc) {
		const svmatch &m = *inc;
		const auto pcmd_start = raw_source.begin() + m.position(0);
		const auto pcmd_end = pcmd_start + m.length(0);

		current_chunk += std::string_view(&*subchunk_begin, pcmd_start - subchunk_begin);
		subchunk_begin = pcmd_end;

		// check for preprocessor definition kind
		// m.str(1) unfortunately returns string, not string_view to a range within raw_source ;_;
		const std::string pcmd = m.str(1);
		UniformType uniform_type;
		enum class PcmdKind {
			Skip, Uniform, Include
		} pcmd_kind = PcmdKind::Skip;
		if (pcmd == "float") {
			uniform_type = UniformType::Float;
			pcmd_kind = PcmdKind::Uniform;
		} else if (pcmd == "vec2") {
			uniform_type = UniformType::Vec2;
			pcmd_kind = PcmdKind::Uniform;
		} else if (pcmd == "vec3") {
			uniform_type = UniformType::Vec3;
			pcmd_kind = PcmdKind::Uniform;
		} else if (pcmd == "vec4") {
			uniform_type = UniformType::Vec4;
			pcmd_kind = PcmdKind::Uniform;
		} else if (pcmd == "include") {
			pcmd_kind = PcmdKind::Include;
		} else if (pcmd == "version") {
			const std::string sver = m.str(2);
			size_t pos = 0;
			version = std::stoi(sver, &pos);
			if (pos != sver.length())
				throw std::runtime_error(format("Unexpected #version number %s", sver.c_str()));
		} else {
			current_chunk += std::string_view(&*pcmd_start, pcmd_end - pcmd_start);
		}

		if (pcmd_kind == PcmdKind::Uniform) {
			// Store new inline uniform in table
			const std::string name = m.str(2);
			const UniformsMap::const_iterator it = uniforms.find(name);
			if (it != uniforms.end()) {
				if (it->second.name != name || it->second.type != uniform_type)
					throw std::runtime_error(format("Type mismatch for variable %s at %s", name.c_str(), m.str(0).c_str()));
			} else {
				uniforms[name] = UniformDeclaration{uniform_type, name};
			}

			current_chunk += name;
		}

		if (pcmd_kind == PcmdKind::Include) {
			std::string chunk;
			std::swap(chunk, current_chunk);
			chunks.emplace_back(Chunk::Type::String, std::move(chunk));

			const auto filename = m.str(2);
			if (filename[0] != '"' || filename[filename.size()-1] != '"')
				throw std::runtime_error(format("#include filename must be in quotes: \"%s\"", filename.c_str()));
			chunks.emplace_back(Chunk::Type::Include, std::string(filename.begin() + 1, filename.end() - 1));
		}

	} // for all preprocessor commands

	const auto subchunk_end = raw_source.end();
	current_chunk += std::string_view(&*subchunk_begin, subchunk_end - subchunk_begin);
	chunks.emplace_back(Chunk::Type::String, std::move(current_chunk));

	return Source(version, std::move(chunks), std::move(uniforms));
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

} // namespace shader
