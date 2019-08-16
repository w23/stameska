#include "PolledShaderSource.h"
#include "Resources.h"
#include "format.h"

PolledShaderSource::PolledShaderSource(Resources &resources, const std::shared_ptr<PolledFile>& file)
	: resources_(resources)
	, file_(file)
{
}

bool PolledShaderSource::poll(unsigned int poll_seq) {
	if (!beginUpdate(poll_seq))
		return false;

	bool need_full_rebuild = false;
	if (file_.poll(poll_seq)) {
		auto load_result = shader::Source::load(file_->string());
		if (!load_result.hasValue()) {
			MSG("Cannot load shader source '%.*s': %s", (int)file_->string().size(), file_->string().data(), load_result.error().c_str());
			return false;
		}

		shader::Source src = std::move(load_result).value();

		std::vector<Chunk> chunks;
		for (const auto& chunk: src.chunks()) {
			switch (chunk.type) {
				case shader::Source::Chunk::Type::String:
					chunks.emplace_back(chunk.value);
					break;
				case shader::Source::Chunk::Type::Include:
					chunks.emplace_back(resources_.getShaderSource(chunk.value));
					break;
				case shader::Source::Chunk::Type::Uniform:
					chunks.emplace_back(Chunk::Uniform(), chunk.value);
					break;
			}
		}

		chunks_ = std::move(chunks);
		uniforms_ = src.uniforms();
		version_ = src.version();
		need_full_rebuild = true;
	}

	for (auto &chunk: chunks_) {
		if (Chunk::Type::Include == chunk.type) {
			if (chunk.include.poll(poll_seq))
				need_full_rebuild = true;
		}
	}

	if (!need_full_rebuild)
		return false;

	shader::UniformsMap uniforms = uniforms_;
	std::string source;
	int version = version_;
	for (const auto& chunk: chunks_) {
		switch (chunk.type) {
			case Chunk::Type::String:
				source += chunk.string;
				break;
			case Chunk::Type::Include:
				if (chunk.include) {
					const auto result = shader::appendUniforms(uniforms, chunk.include->uniforms());
					if (!result.hasValue()) {
						MSG("Cannot load shader source '%.*s': error while merging uniforms: %s", (int)file_->string().size(), file_->string().data(), result.error().c_str());
						return false;
					}
					source += chunk.include->source();
					source += "\n";
					if (chunk.include->version() > version)
						version = chunk.include->version();
				}
				break;
			case Chunk::Type::Uniform:
				source += chunk.string;
				break;
		}
	}

	std::string header = (version != 0) ? format("#version %d\n", version) : "";
	for (const auto &uni: uniforms)
		header += format("uniform %s %s;\n", uniformName(uni.second.type), uni.second.name.c_str());

	source_ = std::move(source);
	header_ = std::move(header);
	uniforms_ = std::move(uniforms);
	version_ = version;

	return endUpdate();
}
