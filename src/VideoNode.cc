#include "VideoNode.h"

#include "PolledPipelineDesc.h"
#include "VideoEngine.h"
#include "Export.h"
#include "Resources.h"
#include "AudioCtl.h" // FIXME Timescope

#include "OpenGL.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "imgui.h"

#define DEF_MODE(X) \
    X(1920, 1080, "1080p") \
    X(1280, 720, "720p") \
    X(640, 360, "360p") \
    X(320, 180, "180p") \

static const char* const c_mode_names[] = {
    #define X(w,h,str) str,
    DEF_MODE(X)
    #undef X
};

static const struct { int w, h; } c_mode_res[] = {
    #define X(w,h,str) {w, h},
    DEF_MODE(X)
    #undef X
};

void VideoNode::resize(int w, int h) {
	preview.w = w;
	preview.h = h;
}

VideoNode::VideoNode(fs::path project_root, std::string_view video_config_filename)
    : INode("VideoNode", "video")
    , preview{1280, 720}
    , polled_pipeline(new PolledPipelineDesc(
        std::shared_ptr<PolledFile>(
            new PolledFile(
                (project_root/video_config_filename).string()))))
    , resources(new Resources(project_root)) {
	GL_LOAD_FUNCS
}

VideoNode::~VideoNode() {}

void VideoNode::paint(float dt, const Timecode& tc, IScope &scope) {
	static unsigned int frame_seq = 0;
	frame_seq++;

	if (polled_pipeline->poll(frame_seq)) {
		engine.reset(new VideoEngine(*resources, polled_pipeline->get()));
		engine->setCanvasResolution(c_mode_res[mode_index].w, c_mode_res[mode_index].h);
	}

	if (engine)
		engine->paint(frame_seq, preview.w, preview.h, tc.row, dt, scope);
}

// void video_export(const ExportSettings &settings, const IAutomation &automation) {
// 	if (polled_pipeline && polled_pipeline->get()) {
// 		auto result = exportC(*resources, settings, *polled_pipeline->get().get(), automation);
// 		if (!result)
// 			MSG("Export error: %s", result.error().c_str());
// 	}
// }

void VideoNode::doUi() noexcept {
    if (ImGui::Combo("Resolution", &mode_index, c_mode_names, COUNTOF(c_mode_names))) {
     	if (engine)
            engine->setCanvasResolution(c_mode_res[mode_index].w, c_mode_res[mode_index].h);
    }

    if (ImGui::Button("Make screensot") && engine) {
        // TODO filename picker
        // TODO incrementable filenames
        // TODO project-relative

        const char *filename = "screenshot.jpg";
        const Image screen = engine->makeScreenshot();
        const int result = stbi_write_jpg(filename, screen.w, screen.h, 3, screen.data.data(), 80);
        MSG("screenshot written to file %s, result: %d", filename, result);
    }
}