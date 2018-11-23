#pragma once

#include "string_view.h"
#include "json.hpp"
#include <memory>
#include <map>

using json = nlohmann::json;
class Timeline;
class PolledShaderSource;
class PolledShaderProgram;

class VideoEngine {
public:
	VideoEngine(string_view config);
	~VideoEngine();

	void poll(unsigned int poll_seq);

	void draw(int w, int h, float row, Timeline &timeline);

private:
	void readPrograms(const json& j);

private:
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_sources_;
	std::map<std::string, std::unique_ptr<PolledShaderProgram>> programs_;
};
