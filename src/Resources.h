#pragma once

#include "filesystem.h"
#include <memory>
#include <string>
#include <map>

class PolledFile;
class PolledShaderSource;
class PolledTexture;

class Resources {
public:
	Resources(const fs::path &project_root) : project_root_(project_root) {}
	std::shared_ptr<PolledShaderSource> getShaderSource(const std::string &filename);
	std::shared_ptr<PolledFile> getFile(const std::string &filename);
	std::shared_ptr<PolledTexture> getTexture(const std::string &filename);

private:
	const fs::path project_root_;
	int shader_include_guard_ = 0;
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_sources_;
	std::map<std::string, std::shared_ptr<PolledFile>> files_;
	std::map<std::string, std::shared_ptr<PolledTexture>> textures_;
};
