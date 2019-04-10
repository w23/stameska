#pragma once

class Timeline;

void video_init(const char *config);
void video_preview_resize(int w, int h);
void video_canvas_resize(int w, int h);
void video_paint(float row, float dt, Timeline &);
void video_export();
