#include "video.h"
#include "VideoEngine.h"
#define GL_DECLARE_FUNC(type, name) type gl ## name;
#include "OpenGL.h"

static struct {
	int w, h;
} g;

void video_tool_resize(int w, int h) {
	g.w = w;
	g.h = h;
	glViewport(0, 0, w, h);
}

void video_init(const char *config) {
	(void)config;
	GL_LOAD_FUNCS
}

void video_paint(float row, Timeline &timeline) {
	static unsigned int poll_seq = 0;
	poll_seq++;

	(void)row; (void)timeline; (void)poll_seq;

	glClearColor(0, 1, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}
