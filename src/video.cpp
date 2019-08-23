#include "video.h"

#define GL_DECLARE_FUNC(type, name) PFNGL ## type ## PROC gl ## name;
#include "OpenGL.h"

#include "PolledPipelineDesc.h"
#include "VideoEngine.h"
#include "Export.h"

#include <memory>

static struct {
	struct { int w, h; } preview;
	struct { int w, h; } canvas;
	std::unique_ptr<PolledPipelineDesc> polled_pipeline;
	std::unique_ptr<VideoEngine> engine;
} g;

void video_canvas_resize(int w, int h) {
	g.canvas.w = w;
	g.canvas.h = h;
	if (g.engine)
		g.engine->setCanvasResolution(w, h);
}

void video_preview_resize(int w, int h) {
	g.preview.w = w;
	g.preview.h = h;
}

void video_init(const char *config) {
	GL_LOAD_FUNCS

	g.canvas.w = 1280;
	g.canvas.h = 720;

	g.polled_pipeline.reset(new PolledPipelineDesc(
			std::shared_ptr<PolledFile>(new PolledFile(config))));
}

void video_paint(float row, float dt, IScope &scope) {
	static unsigned int frame_seq = 0;
	frame_seq++;

	if (g.polled_pipeline->poll(frame_seq)) {
		g.engine.reset(new VideoEngine(g.polled_pipeline->get()));
		g.engine->setCanvasResolution(g.canvas.w, g.canvas.h);
	}

	if (g.engine)
		g.engine->paint(frame_seq, g.preview.w, g.preview.h, row, dt, scope);
}

void video_export(const ExportSettings &settings, const IAutomation &automation) {
	if (g.polled_pipeline && g.polled_pipeline->get()) {
		auto result = exportC(settings, *g.polled_pipeline->get().get(), automation);
		if (!result)
			MSG("Export error: %s", result.error().c_str());
	}

}
