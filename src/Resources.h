#pragma once

#include <memory>
#include <string>
#include <map>
#include <filesystem>

class PolledShaderSource;

class Resources {
public:
	Resources(const std::filesystem::path &project_root) : project_root_(project_root) {}
	std::shared_ptr<PolledShaderSource> getShaderSource(const std::string &filename);

private:
	const std::filesystem::path project_root_;
	int shader_include_guard_ = 0;
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_sources_;
};
