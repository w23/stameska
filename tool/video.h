#pragma once

#define SHADERS(S) \
	S(blitter) \
	S(intro)

#define PROGRAMS(P) \
	P(intro, intro) \
	P(blitter, blitter)

void video_init(int w, int h);
void video_paint(float tick);
