#include "video.h"
#include "VideoEngine.h"
#include "PolledFile.h"
#include "Timeline.h"
#include "utils.h"
#define GL_DECLARE_FUNC(type, name) type gl ## name;
#include "OpenGL.h"

#include <memory>

static struct {
	struct { int w = 1920, h = 1080; } output;

	std::shared_ptr<PolledFile> config_file;
	std::unique_ptr<VideoEngine> engine;
} state;

void video_tool_resize(int w, int h) {
	state.output.w = w;
	state.output.h = h;
	glViewport(0, 0, w, h);
}

void video_init(const char *config) {
	GL_LOAD_FUNCS

	state.config_file.reset(new PolledFile(config));
}

void video_paint(float row, Timeline &timeline) {
	static unsigned int poll_seq = 0;
	poll_seq++;

	if (state.config_file->poll(poll_seq)) {
		try {
			state.engine.reset(new VideoEngine(state.config_file->string()));
		}
		catch (const std::exception& e) {
			MSG("Error updating video configuration: %s", e.what());
		}
	}

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (state.engine) {
		state.engine->poll(poll_seq);
		state.engine->draw(state.output.w, state.output.h, row, timeline);
	}
}
