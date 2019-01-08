#pragma once

#include <memory>
#include <string>
#include <map>

class PolledShaderSource;

class Resources {
public:
	std::shared_ptr<PolledShaderSource> getShaderSource(const std::string &filename);

private:
	int shader_include_guard_ = 0;
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_sources_;
};
