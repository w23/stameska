#include "Program.h"
#include "VideoEngine.h"
#include "RenderDesc.h"
#include "PolledShaderProgram.h"
#include "PolledTexture.h"
#include "Variables.h"
#include "format.h"

#include <set>

class VideoEngine::Canvas {
public:
	Canvas(int w, int h) {
		color_.alloc(w, h, RGBA8);
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fb_.name));

		GLuint depth_renderbuffer;
		GL(glGenRenderbuffers(1, &depth_renderbuffer));
		GL(glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer));
		GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
		GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer));
		//GL(glDeleteRenderbuffers(1, &depth_renderbuffer));

		fb_.attachColorTexture(0, color_);

		auto p = Program::create(
			"#version 130\n"
			"uniform sampler2D frame;\n"
			"uniform vec2 R;\n"
			"void main() {\n"
				"vec2 ts = vec2(textureSize(frame, 0));\n"
				"vec2 k = ts / R;\n"
				"float scale = max(k.x, k.y);\n"
				"vec2 off = (R-ts/scale)/2. * vec2(step(k.x, k.y), step(k.y, k.x));\n"
				"vec2 tc = scale * (gl_FragCoord.xy - off);\n"
				"gl_FragColor = vec4(0.);\n"
				"if (tc.x >= 0. && tc.x < ts.x && tc.y >= 0. && tc.y < ts.y)\n"
					"gl_FragColor = texture2D(frame, tc / (ts + vec2(1.)));\n"
			"}\n",
			"void main() { gl_Position = gl_Vertex; }");

		if (!p)
			CRASH("Cannot create canvas program: %s", p.error().c_str());

		program_ = std::move(p).value();
	}

	const Program &program() const { return program_; }
	const Texture &color() const { return color_; }
	const Framebuffer &framebuffer() const { return fb_; }

	int width() const { return fb_.w; }
	int height() const { return fb_.h; }

private:
	Program program_;
	Texture color_;
	Framebuffer fb_;
};

bool VideoEngine::Framebuffer::attachColorTexture(int i, const Texture &tex) {
	// FIXME check?
	w = tex.w();
	h = tex.h();
	GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex.name(), 0));

	const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

#define MAX_PASS_TEXTURES 4

#ifndef ATTO_PLATFORM_RPI
static const GLuint draw_buffers[MAX_PASS_TEXTURES] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};
#endif

static Expected<std::string, std::string> shaderPreprocessor(const shader::Source &flat) {
	std::string source;
	if (flat.version() != 0)
		source = format("#version %d\n", flat.version());
	for (const auto &[name, decl]: flat.uniforms()) {
		switch(decl.type) {
			case shader::UniformType::Float:
				source += "uniform float ";
				break;
			case shader::UniformType::Vec2:
				source += "uniform vec2 ";
				break;
			case shader::UniformType::Vec3:
				source += "uniform vec3 ";
				break;
			case shader::UniformType::Vec4:
				source += "uniform vec4 ";
				break;
		}
		source += name + ";";
	}

	for (const auto &chunk: flat.chunks()) {
		switch (chunk.type) {
			case shader::Source::Chunk::Type::Uniform:
			case shader::Source::Chunk::Type::String:
				source += chunk.value;
				break;
			case shader::Source::Chunk::Type::Include:
				return Unexpected<std::string>("Include chunk is invalid in flat shader source");
		}
	}

	return Expected<std::string, std::string>(std::move(source));
}

VideoEngine::VideoEngine(Resources &resources, const std::shared_ptr<renderdesc::Pipeline> &pipeline)
	: resources_(resources)
	, pipeline_(pipeline)
{
	for (const auto& t: pipeline->textures) {
		if (t.file.empty()) {
			textures_.emplace_back(std::shared_ptr<PolledTexture>(new PolledTexture(t.w, t.h, t.pixel_type)));
		} else {
			textures_.emplace_back(resources_.getTexture(t.file));
		}
	}

	for (const auto& f: pipeline->framebuffers) {
		Framebuffer fb;
		GL(glBindFramebuffer(GL_FRAMEBUFFER, fb.name));
		for (int i = 0; i < f.textures_count; ++i) {
			const int index = f.textures[i];
			if (index < 0 || index >= (int)textures_.size())
				CRASH("Fb texture %d OOB %d (max %d)", i, index, (int)textures_.size());

			if (!fb.attachColorTexture(i, textures_[index]->texture()))
				CRASH("Framebuffer %d is not complete", (int)framebuffer_.size());
		}

		fb.num_targets = f.textures_count;

		framebuffer_.push_back(std::move(fb));
	}

	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	for (const auto& s: pipeline->shader_filenames)
		sources_.emplace_back(resources_.getShaderSource(s));

	for (const auto& p: pipeline->programs) {
		if (p.vertex < 0 || p.vertex >= (int)sources_.size())
			CRASH("Program vertex source OOB %d (max %d)", p.vertex, (int)sources_.size());
		if (p.fragment < 0 || p.fragment >= (int)sources_.size())
			CRASH("Program fragment source OOB %d (max %d)", p.fragment, (int)sources_.size());

		programs_.emplace_back(shaderPreprocessor, sources_[p.vertex], sources_[p.fragment]);
	}
}

VideoEngine::~VideoEngine() {
}

static const std::set<std::string> internal_uniforms = {"R", "t"};

static void useProgram(const PolledShaderProgram& program, int w, int h, float row, float dt, IScope &scope) {
	const Program& p = program.get();
	if (!p.valid())
		return;

	p.use();

	for (const auto &it: program.uniforms()) {
		if (internal_uniforms.find(it.first) != internal_uniforms.end())
			continue;

		const Value v = scope.getValue(it.first, static_cast<int>(it.second.type));
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

	p.setUniform("R", w, h).setUniform("t", row).setUniform("dt", dt);
}

void VideoEngine::setCanvasResolution(int w, int h) {
#ifndef ATTO_PLATFORM_RPI
	canvas_.reset(new Canvas(w, h));
#else
	(void)w;
	(void)h;
#endif
}

void VideoEngine::paint(unsigned int frame_seq, int preview_width, int preview_height, float row, float dt, IScope &scope) {
	const int pingpong[3] = {0, (int)(frame_seq & 1), (int)((frame_seq + 1) & 1)};

	for (auto &p: programs_)
		p.poll(frame_seq);

	for (auto &t: textures_)
		t->poll(frame_seq);

	glViewport(0, 0, preview_width, preview_height);
	glClear(GL_COLOR_BUFFER_BIT);

	// FIXME
	glEnable(GL_POINT_SPRITE);

#ifndef ATTO_PLATFORM_RPI
	if (!canvas_) {
		glViewport(0, 0, preview_width, preview_height);
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}
#endif

	struct {
		int w, h;
		const Program *program = nullptr;
		// TODO better texture tracking mechanism to support texture changes between draw calls
		int first_availabale_texture_slot = 0;
	} runtime = {
		canvas_ ? canvas_->width() : preview_width,
		canvas_ ? canvas_->height() : preview_height
	};

#ifndef ATTO_PLATFORM_RPI
	GL(glBindFramebuffer(GL_FRAMEBUFFER, canvas_->framebuffer().name));
#else
	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
#endif
	glViewport(0, 0, runtime.w, runtime.h);

	for (const auto &cmd: pipeline_->commands) {
		switch (cmd.op) {
			case renderdesc::Command::Op::BindFramebuffer:
				{
					const renderdesc::Command::BindFramebuffer &cmdfb = cmd.bindFramebuffer;
					if (cmdfb.framebuffer.index == -1) {
						if (canvas_) {
							GL(glBindFramebuffer(GL_FRAMEBUFFER, canvas_->framebuffer().name));
							runtime.w = canvas_->width();
							runtime.h = canvas_->height();
						} else {
							GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
							runtime.w = preview_width;
							runtime.h = preview_height;
						}
					} else {
						const int index = cmdfb.framebuffer.index + pingpong[cmdfb.framebuffer.pingpong];

						// FIXME validate index

						const Framebuffer &fb = framebuffer_[index];
						GL(glBindFramebuffer(GL_FRAMEBUFFER, fb.name));
#ifndef ATTO_PLATFORM_RPI
						GL(glDrawBuffers(fb.num_targets, draw_buffers));
#endif
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
					useProgram(prog, runtime.w, runtime.h, row, dt, scope);
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
					textures_[index]->texture().bind(slot);
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
#ifndef ATTO_PLATFORM_RPI
					case renderdesc::Command::Flag::VertexProgramPointSize:
						glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
						break;
#endif
				}
				break;
			case renderdesc::Command::Op::Disable:
				switch (cmd.disable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						glDisable(GL_DEPTH_TEST);
						break;
#ifndef ATTO_PLATFORM_RPI
					case renderdesc::Command::Flag::VertexProgramPointSize:
						glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
						break;
#endif
				}
				break;
			case renderdesc::Command::Op::DrawArrays:
				GL(glDrawArrays((GLenum)cmd.drawArrays.mode, cmd.drawArrays.start, cmd.drawArrays.count));
				break;
			case renderdesc::Command::Op::DrawFullscreen:
#ifndef ATTO_PLATFORM_RPI
				GL(glRects(-1,-1,1,1));
#else
// FIXME
#endif
				break;
		}
	}

#ifndef ATTO_PLATFORM_RPI
	GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	glViewport(0, 0, preview_width, preview_height);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, canvas_->color().name());
	canvas_->program().use().setUniform("R", preview_width, preview_height).setUniform("frame", 0);
	glRects(-1,-1,1,1);
#endif
} // void VideoEngine::paint()

Image VideoEngine::makeScreenshot() const {
	std::vector<unsigned char> data((size_t)(canvas_->width() * canvas_->height() * 3), 0);
	glReadPixels(0, 0, canvas_->width(), canvas_->height(), GL_RGB, GL_UNSIGNED_BYTE, data.data());
	return Image{canvas_->width(), canvas_->height(), std::move(data)};
}
