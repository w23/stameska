#include "Resources.h"
#include "PolledShaderSource.h"
#include "PolledFile.h"
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
		new PolledShaderSource(*this,
			std::shared_ptr<PolledFile>(new PolledFile((project_root_/filename).string()))));
	shader_sources_[filename] = source;
	return source;
}
