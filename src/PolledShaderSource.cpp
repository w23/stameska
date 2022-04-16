#include "PolledShaderSource.h"
#include "Resources.h"
#include "format.h"

PolledShaderSource::PolledShaderSource(Resources &resources, const std::shared_ptr<PolledFile>& file)
	: resources_(resources)
	, file_(file)
{
	if (readSources())
		constructShader();
}

bool PolledShaderSource::readSources() {
		MSG("Loaded %.*s", PRISV(file_->string()));
		auto load_result = shader::Source::load(file_->string());
		if (!load_result.hasValue()) {
			MSG("Cannot load shader source '%.*s': %s", PRISV(file_->string()), load_result.error().c_str());
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

		version_ = src.version();
		chunks_ = std::move(chunks);
		uniforms_ = src.uniforms();
		return true;
}

bool PolledShaderSource::constructShader() {
	std::vector<shader::Source::Chunk> flat_chunks;
	shader::UniformsMap uniforms = uniforms_;
	int version = version_;
	for (const auto& chunk: chunks_) {
		switch (chunk.type) {
			case Chunk::Type::String:
				flat_chunks.emplace_back(shader::Source::Chunk::Type::String, chunk.string);
				break;
			case Chunk::Type::Uniform:
				flat_chunks.emplace_back(shader::Source::Chunk::Type::Uniform, chunk.string);
				break;
			case Chunk::Type::Include:
				if (chunk.include) {
					const shader::Source &inc_source = chunk.include->flatSource();
					const auto result = shader::appendUniforms(uniforms, inc_source.uniforms());
					if (!result.hasValue()) {
						MSG("Cannot load shader source '%.*s': error while merging uniforms: %s", PRISV(file_->string()), result.error().c_str());
						return false;
					}

					if (inc_source.version() > version)
						version = inc_source.version();

					const std::vector<shader::Source::Chunk> &inc_chunks = inc_source.chunks();
					flat_chunks.insert(flat_chunks.end(), inc_chunks.begin(), inc_chunks.end());
				}
				break;
		}
	}

	version_ = version;
	flat_source_ = shader::Source(version, std::move(flat_chunks), std::move(uniforms));
	return true;
}

bool PolledShaderSource::poll(unsigned int poll_seq) {
	if (!beginUpdate(poll_seq))
		return false;

	bool need_full_rebuild = false;
	if (file_.poll(poll_seq)) {
		if (!readSources())
			return false;

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

	constructShader();

	return endUpdate();
}
