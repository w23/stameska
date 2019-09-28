#pragma once

#include <filesystem>
#include <string_view>

class IScope;
class IAutomation;
struct ExportSettings;

void video_init(std::filesystem::path project_root, std::string_view video_config_filename);
void video_preview_resize(int w, int h);
void video_canvas_resize(int w, int h);
void video_paint(float row, float dt, IScope &);
void video_export(const ExportSettings &, const IAutomation &);
