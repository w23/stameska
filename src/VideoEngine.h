#pragma once

#include "Resources.h"
#include "OpenGL.h"

#include <memory>
#include <vector>

namespace renderdesc { class Pipeline; }
class PolledShaderProgram;
class PolledShaderSource;
class Timeline;

class VideoEngine {
public:
	VideoEngine(const std::shared_ptr<renderdesc::Pipeline> &pipeline);
	~VideoEngine();

	void paint(unsigned int frame_seq, int w, int h, float row, Timeline &timeline);

private:
	struct Framebuffer {
		int w, h;
		int num_targets;
		GLuint name;
	};

	const std::shared_ptr<renderdesc::Pipeline> pipeline_;
	Resources resources_;
	std::vector<GLuint> textures_;
	std::vector<Framebuffer> framebuffer_;
	std::vector<std::shared_ptr<PolledShaderSource>> sources_;
	std::vector<PolledShaderProgram> programs_;
};
