#pragma once

class IScope;
struct ExportSettings;

void video_init(const char *config);
void video_preview_resize(int w, int h);
void video_canvas_resize(int w, int h);
void video_paint(float row, float dt, IScope &);
void video_export(const ExportSettings &);
