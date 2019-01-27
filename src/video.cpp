#include "video.h"

#define GL_DECLARE_FUNC(type, name) type gl ## name;
#include "OpenGL.h"

#include "RenderDesc.h"
#include "VideoEngine.h"

#include <memory>

static struct {
	int w, h;
	std::unique_ptr<VideoEngine> engine;
} g;

void video_tool_resize(int w, int h) {
	g.w = w;
	g.h = h;
}

void video_init(const char *config) {
	(void)config;

	GL_LOAD_FUNCS

	const std::shared_ptr<renderdesc::Pipeline> pipeline(new renderdesc::Pipeline());

	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);
	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);
	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);
	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);
	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);
	pipeline->textures.emplace_back(512, 512, renderdesc::Texture::RGBA32F);

	renderdesc::Framebuffer fb;
	fb.textures_count = 3;
	fb.textures[0] = 0;
	fb.textures[1] = 2;
	fb.textures[2] = 4;
	pipeline->framebuffers.push_back(fb);
	fb.textures[0] = 1;
	fb.textures[1] = 3;
	fb.textures[2] = 5;
	pipeline->framebuffers.push_back(fb);

	pipeline->shader_filenames = {
		"puvtx.glsl",
		"pufrag.glsl",
		"pdrawvtx.glsl",
		"pdrawfrag.glsl",
	};

	pipeline->programs.emplace_back(0, 1);
	pipeline->programs.emplace_back(2, 3);

	pipeline->commands.emplace_back(renderdesc::Command::BindFramebuffer(renderdesc::Command::Index(0, renderdesc::Command::Index::Ping)));
	pipeline->commands.emplace_back(renderdesc::Command::Clear(0, 0, 0, 0, 0));
	pipeline->commands.emplace_back(renderdesc::Command::UseProgram(0));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P0", renderdesc::Command::Index(0, renderdesc::Command::Index::Pong)));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P1", renderdesc::Command::Index(2, renderdesc::Command::Index::Pong)));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P2", renderdesc::Command::Index(4, renderdesc::Command::Index::Pong)));
	pipeline->commands.emplace_back(renderdesc::Command::DrawFullscreen());

	pipeline->commands.emplace_back(renderdesc::Command::BindFramebuffer(renderdesc::Command::Index(-1)));
	pipeline->commands.emplace_back(renderdesc::Command::UseProgram(1));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P0", renderdesc::Command::Index(0, renderdesc::Command::Index::Ping)));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P1", renderdesc::Command::Index(2, renderdesc::Command::Index::Ping)));
	pipeline->commands.emplace_back(renderdesc::Command::BindTexture("P2", renderdesc::Command::Index(4, renderdesc::Command::Index::Ping)));
	pipeline->commands.emplace_back(renderdesc::Command::Enable(renderdesc::Command::Flag::DepthTest));
	pipeline->commands.emplace_back(renderdesc::Command::Enable(renderdesc::Command::Flag::VertexProgramPointSize));
	pipeline->commands.emplace_back(renderdesc::Command::Clear(0, 0, 0, 0, true));
	pipeline->commands.emplace_back(renderdesc::Command::DrawArrays(0, 512 * 512));
	pipeline->commands.emplace_back(renderdesc::Command::Disable(renderdesc::Command::Flag::DepthTest));
	pipeline->commands.emplace_back(renderdesc::Command::Disable(renderdesc::Command::Flag::VertexProgramPointSize));

	g.engine.reset(new VideoEngine(pipeline));
}

void video_paint(float row, Timeline &timeline) {
	if (g.engine)
		g.engine->paint(g.w, g.h, row, timeline);
}
