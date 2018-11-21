#pragma once

class Timeline;

void video_init(int w, int h, const char *shader);
void video_tool_resize(int w, int h);
void video_paint(float tick, Timeline &);
