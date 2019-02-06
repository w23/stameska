#include "video.h"

#define GL_DECLARE_FUNC(type, name) PFNGL ## type ## PROC gl ## name;
#include "OpenGL.h"

#include "PolledPipelineDesc.h"
#include "VideoEngine.h"
#include "Export.h"

#include <memory>

static struct {
	int w, h;
	std::unique_ptr<PolledPipelineDesc> polled_pipeline;
	std::unique_ptr<VideoEngine> engine;
} g;

void video_tool_resize(int w, int h) {
	g.w = w;
	g.h = h;
}

void video_init(const char *config) {
	GL_LOAD_FUNCS

	g.polled_pipeline.reset(new PolledPipelineDesc(
			std::shared_ptr<PolledFile>(new PolledFile(config))));
}

void video_paint(float row, Timeline &timeline) {
	static unsigned int frame_seq = 0;
	frame_seq++;

	if (g.polled_pipeline->poll(frame_seq))
		g.engine.reset(new VideoEngine(g.polled_pipeline->get()));

	if (g.engine)
		g.engine->paint(frame_seq, g.w, g.h, row, timeline);
}

void video_export() {
	if (g.polled_pipeline && g.polled_pipeline->get())
		exportC(*g.polled_pipeline->get().get(), 1280, 720, "stameska_export.c");

}
