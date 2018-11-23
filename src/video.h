#pragma once

class Timeline;

void video_init(const char *config);
void video_tool_resize(int w, int h);
void video_paint(float row, Timeline &);
