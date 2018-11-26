#include "video.h"
#include "VideoEngine.h"
#define GL_DECLARE_FUNC(type, name) type gl ## name;
#include "OpenGL.h"

static struct {
	int w, h;
	VideoEngine engine;
	std::shared_ptr<PolledShaderProgram> prog;
} g;

void video_tool_resize(int w, int h) {
	g.w = w;
	g.h = h;
	glViewport(0, 0, w, h);
}

void video_init(const char *config) {
	GL_LOAD_FUNCS

	g.prog = g.engine.getFragmentProgramWithShaders(140, "main", {config});
}

void video_paint(float row, Timeline &timeline) {
	static unsigned int poll_seq = 0;
	poll_seq++;

	g.prog->poll(poll_seq);

	glClearColor(0, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	g.engine.useProgram(*g.prog.get(), g.w, g.h, row, timeline);
	if (g.prog->get().valid())
		g.prog->get().compute();
}
