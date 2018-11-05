#pragma once

#define SHADERS(S) \
	S(blitter) \
	S(intro)

#define PROGRAMS(P) \
	P(intro, intro) \
	P(blitter, blitter)

void video_init(int w, int h);
#ifdef TOOL
void video_tool_resize(int w, int h);
#endif
void video_paint(float tick);
