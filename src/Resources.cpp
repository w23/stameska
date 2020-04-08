#include "Resources.h"
#include "PolledFile.h"
#include "PolledShaderSource.h"
#include "PolledTexture.h"
#include "utils.h"

std::shared_ptr<PolledShaderSource> Resources::getShaderSource(const std::string &filename) {
	struct AutoInc {
		int &i;
		AutoInc(int &i) : i(i) { ++i; }
		~AutoInc() { --i; }
	};

	const AutoInc include_inc(shader_include_guard_);
	if (shader_include_guard_ > 10) {
		MSG("Include is too deep at file %s", filename.c_str());
		return std::shared_ptr<PolledShaderSource>();
	}

	const auto it = shader_sources_.find(filename);
	if (it != shader_sources_.end())
		return it->second;

	const std::shared_ptr<PolledShaderSource> source(
		new PolledShaderSource(*this, getFile(filename)));
	shader_sources_[filename] = source;
	return source;
}

std::shared_ptr<PolledFile> Resources::getFile(const std::string &filename) {
	const auto it = files_.find(filename);
	if (it != files_.end())
		return it->second;

	auto file = std::shared_ptr<PolledFile>(new PolledFile((project_root_/filename).string()));
	files_[filename] = file;
	return file;
}

std::shared_ptr<PolledTexture> Resources::getTexture(const std::string &filename) {
	const auto it = textures_.find(filename);
	if (it != textures_.end())
		return it->second;

	auto texture = std::shared_ptr<PolledTexture>(new PolledTexture(getFile(filename)));
	textures_[filename] = texture;
	return texture;
}
