#pragma once

#include "filesystem.h"
#include <string_view>
#include <memory>

class IScope;
struct Timecode;
class VideoEngine;
class Resources;
class PolledPipelineDesc;

class VideoNode {
public:
    VideoNode(fs::path project_root, std::string_view video_config_filename);
    ~VideoNode();

    void resize(int w, int h);
    void paint(float dt, const Timecode& tc, IScope& automation);
    void paintUi();

private:
    int mode_index = 1;
	struct { int w, h; } preview;
	std::unique_ptr<PolledPipelineDesc> polled_pipeline;
	std::unique_ptr<Resources> resources;
	std::unique_ptr<VideoEngine> engine;
};