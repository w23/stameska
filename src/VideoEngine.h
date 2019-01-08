#pragma once

#include "PolledResource.h"
#include "PolledFile.h"
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
	VideoEngine() {}
	VideoEngine(string_view config);
	~VideoEngine();

	void poll(unsigned int poll_seq);

	void draw(int w, int h, float row, Timeline &timeline);

	std::shared_ptr<PolledShaderProgram> getFragmentProgramWithShaders(int version, const std::string &name,
			const std::vector<std::string> &fragment);
	std::shared_ptr<PolledShaderProgram> getProgramWithShaders(int version, const std::string &name,
			const std::vector<std::string> &vertex, const std::vector<std::string> &fragment);
	void useProgram(PolledShaderProgram& program, int w, int h, float row, Timeline &timeline);

private:
	void readPrograms(const json& j);
	void readShaders(const json& j);

	std::shared_ptr<PolledShaderSource> getPolledShaderForFiles(int version, const std::vector<std::string> &vertex);

private:
	std::map<std::string, std::shared_ptr<PolledShaderSource>> shader_source_;
	std::map<std::string, std::shared_ptr<PolledShaderProgram>> program_;
};
