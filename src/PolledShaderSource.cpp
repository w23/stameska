#include "PolledShaderSource.h"
#include "Resources.h"

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
		try {
			shader::Source src = shader::Source::load(file_->string());

			std::vector<Chunk> chunks;
			for (const auto& chunk: src.chunks()) {
				switch (chunk.type) {
					case shader::Source::Chunk::Type::String:
						chunks.emplace_back(chunk.value);
						break;
					case shader::Source::Chunk::Type::Include:
						chunks.emplace_back(resources_.getShaderSource(chunk.value));
						break;
				}
			}

			chunks_ = std::move(chunks);
			uniforms_ = src.uniforms();
			version_ = src.version();
			need_full_rebuild = true;
		}	catch (const std::runtime_error& e) {
			MSG("Error updating shader source: %s", e.what());
			return false;
		}
	}

	for (auto &chunk: chunks_) {
		if (Chunk::Type::Include == chunk.type) {
			if (chunk.include->poll(poll_seq))
				need_full_rebuild = true;
		}
	}

	if (!need_full_rebuild)
		return false;

	try {
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
						shader::appendUniforms(uniforms, chunk.include->uniforms());
						source += chunk.include->source();
						source += "\n";
						if (chunk.include->version() > version)
							version = chunk.include->version();
					}
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
	}	catch (const std::runtime_error& e) {
		MSG("Error updating shader source: %s", e.what());
		return false;
	}

	return endUpdate();
}
