#include "Program.h"
#include "VideoEngine.h"
#include "RenderDesc.h"
#include "PolledShaderProgram.h"
#include "Timeline.h"

#include <set>

#define MAX_PASS_TEXTURES 4

static const GLuint draw_buffers[MAX_PASS_TEXTURES] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};

VideoEngine::VideoEngine(const std::shared_ptr<renderdesc::Pipeline> &pipeline)
	: pipeline_(pipeline)
{
	for (const auto& t: pipeline->textures) {
		int comp = 0, type = 0;
		switch (t.pixel_type) {
			case renderdesc::Texture::RGBA8:
				comp = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				break;
			case renderdesc::Texture::RGBA16F:
				comp = GL_RGBA16F;
				type = GL_FLOAT;
				break;
			case renderdesc::Texture::RGBA32F:
				comp = GL_RGBA32F;
				type = GL_FLOAT;
				break;
		}

		GLuint tex;
		GL(glGenTextures(1, &tex));
		GL(glBindTexture(GL_TEXTURE_2D, tex));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, comp, t.w, t.h, 0, GL_RGBA, type, nullptr));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		/*
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		*/

		textures_.push_back(tex);
	}

	for (const auto& f: pipeline->framebuffers) {
		Framebuffer fb;
		glGenFramebuffers(1, &fb.name);
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fb.name));
		for (int i = 0; i < f.textures_count; ++i) {
			const int index = f.textures[i];
			if (index < 0 || index >= (int)textures_.size()) {
				// FIXME fb handle leaks
				throw std::runtime_error(format("Fb texture %d OOB %d (max %d)",
					i, index, (int)textures_.size()));
			}

			GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures_[index], 0));

			// FIXME check completeness

			// FIXME validate
			fb.w = pipeline->textures[index].w;
			fb.h = pipeline->textures[index].h;
		}
		fb.num_targets = f.textures_count;

		framebuffer_.push_back(std::move(fb));
	}

	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	for (const auto& s: pipeline->shader_filenames)
		sources_.emplace_back(new PolledShaderSource(resources_, std::shared_ptr<PolledFile>(new PolledFile(s))));

	for (const auto& p: pipeline->programs) {
		if (p.vertex < 0 || p.vertex >= (int)sources_.size())
			throw std::runtime_error(format("Program vertex source OOB %d (max %d)", p.vertex, (int)sources_.size()));
		if (p.fragment < 0 || p.fragment >= (int)sources_.size())
			throw std::runtime_error(format("Program fragment source OOB %d (max %d)", p.fragment, (int)sources_.size()));

		programs_.emplace_back(sources_[p.vertex], sources_[p.fragment]);
	}
}

VideoEngine::~VideoEngine() {
}

static const std::set<std::string> internal_uniforms = {"R", "t"};

static void useProgram(const PolledShaderProgram& program, int w, int h, float row, Timeline &timeline) {
	const Program& p = program.get();
	if (!p.valid())
		return;

	p.use();

	for (const auto &it: program.uniforms()) {
		if (internal_uniforms.find(it.first) != internal_uniforms.end())
			continue;

		const Value v = timeline.getValue(it.first, static_cast<int>(it.second.type) + 1);
		switch (it.second.type) {
			case shader::UniformType::Float:
				p.setUniform(it.first.c_str(), v.x);
				break;
			case shader::UniformType::Vec2:
				p.setUniform(it.first.c_str(), v.x, v.y);
				break;
			case shader::UniformType::Vec3:
				p.setUniform(it.first.c_str(), v.x, v.y, v.z);
				break;
			case shader::UniformType::Vec4:
				p.setUniform(it.first.c_str(), v.x, v.y, v.z, v.w);
				break;
		}
	}

	p.setUniform("R", w, h).setUniform("t", row);
}

void VideoEngine::paint(unsigned int frame_seq, int w, int h, float row, Timeline &timeline) {
	const int pingpong[3] = {0, (int)(frame_seq & 1), (int)((frame_seq + 1) & 1)};

	for (auto &p: programs_)
		p.poll(frame_seq);

	struct {
		int w, h;
		const Program *program = nullptr;
		// TODO better texture tracking mechanism to support texture changes between draw calls
		int first_availabale_texture_slot = 0;
	} runtime;
	runtime.w = w;
	runtime.h = h;

	for (const auto &cmd: pipeline_->commands) {
		switch (cmd.op) {
			case renderdesc::Command::Op::BindFramebuffer:
				{
					const renderdesc::Command::BindFramebuffer &cmdfb = cmd.bindFramebuffer;
					if (cmdfb.framebuffer.index == -1) {
						GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
						runtime.w = w;
						runtime.h = h;
					} else {
						const int index = cmdfb.framebuffer.index + pingpong[cmdfb.framebuffer.pingpong];

						// FIXME validate index

						const Framebuffer &fb = framebuffer_[index];
						GL(glBindFramebuffer(GL_FRAMEBUFFER, fb.name));
						GL(glDrawBuffers(fb.num_targets, draw_buffers));
						runtime.w = fb.w;
						runtime.h = fb.h;
					}
					glViewport(0, 0, runtime.w, runtime.h);
					break;
				}

			case renderdesc::Command::Op::UseProgram:
				{
					const renderdesc::Command::UseProgram &cmdp = cmd.useProgram;
					// FIXME validate index
					const PolledShaderProgram &prog = programs_[cmdp.program.index];
					useProgram(prog, runtime.w, runtime.h, row, timeline);
					runtime.program = &prog.get();
					runtime.first_availabale_texture_slot = 0; // TODO bound textures will leak
					break;
				}

			case renderdesc::Command::Op::BindTexture:
				{
					if (!runtime.program || !runtime.program->valid())
						break;

					const auto &cmdtex = cmd.bindTexture;
					const int index = cmdtex.texture.index + pingpong[cmdtex.texture.pingpong];
					// FIXME validate index
					const int slot = (runtime.first_availabale_texture_slot++);
					GL(glActiveTexture(GL_TEXTURE0 + slot));
					GL(glBindTexture(GL_TEXTURE_2D, textures_[index]));
					runtime.program->setUniform(cmdtex.name.c_str(), slot);
					break;
				}

			case renderdesc::Command::Op::Clear:
				{
					const auto &cmdc = cmd.clear;
					glClearColor(cmdc.r, cmdc.g, cmdc.b, cmdc.a);
					glClear(GL_COLOR_BUFFER_BIT | (cmdc.depth?GL_DEPTH_BUFFER_BIT:0));
					break;
				}

			case renderdesc::Command::Op::Enable:
				switch (cmd.enable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						glEnable(GL_DEPTH_TEST);
						break;
					case renderdesc::Command::Flag::VertexProgramPointSize:
						glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
						break;
				}
				break;
			case renderdesc::Command::Op::Disable:
				switch (cmd.enable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						glDisable(GL_DEPTH_TEST);
						break;
					case renderdesc::Command::Flag::VertexProgramPointSize:
						glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
						break;
				}
				break;
			case renderdesc::Command::Op::DrawArrays:
				GL(glDrawArrays((GLenum)cmd.drawArrays.mode, cmd.drawArrays.start, cmd.drawArrays.count));
				break;
			case renderdesc::Command::Op::DrawFullscreen:
				GL(glRects(-1,-1,1,1));
				break;
		}
	}
}
