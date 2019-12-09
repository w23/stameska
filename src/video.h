#pragma once

#include "filesystem.h"
#include <string_view>

class IScope;
class IAutomation;
struct ExportSettings;
namespace FFT { struct Frame; }

void video_init(fs::path project_root, std::string_view video_config_filename);
void video_preview_resize(int w, int h);
void video_canvas_resize(int w, int h);
void video_paint(float row, float dt, IScope &, const FFT::Frame &f);
void video_export(const ExportSettings &, const IAutomation &);
