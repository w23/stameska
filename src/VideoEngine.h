#pragma once

#include "Resources.h"
#include "OpenGL.h"

#include <memory>
#include <vector>

namespace renderdesc { class Pipeline; }
class PolledShaderProgram;
class PolledShaderSource;
class Timeline;
class Texture;

class VideoEngine {
public:
	VideoEngine(const std::shared_ptr<renderdesc::Pipeline> &pipeline);
	~VideoEngine();

	void paint(unsigned int frame_seq, int w, int h, float row, Timeline &timeline);

private:
	struct Framebuffer {
		int w = 0, h = 0;
		int num_targets = 0;
		GLuint name = 0;

		Framebuffer() { glGenFramebuffers(1, &name); }
		Framebuffer(const Framebuffer &) = delete;
		Framebuffer(Framebuffer &&f)
			: w(f.w), h(f.h), num_targets(f.num_targets), name(f.name) { f.name = 0; }
		~Framebuffer() { glDeleteFramebuffers(1, &name); }
	};

	const std::shared_ptr<renderdesc::Pipeline> pipeline_;
	Resources resources_;
	std::vector<Texture> textures_;
	std::vector<Framebuffer> framebuffer_;
	std::vector<std::shared_ptr<PolledShaderSource>> sources_;
	std::vector<PolledShaderProgram> programs_;
};
