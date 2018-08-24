#include <stdio.h>

#include "video.h"
#include "atto/app.h"
#include "atto/platform.h"
//#define ATTO_UDIO_SAMPLERATE 44100
//#define ATTO_UDIO_H_IMPLEMENT
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#ifndef WIDTH
#define WIDTH 1920
#endif
#ifndef HEIGHT
#define HEIGHT 1080
#endif

static struct {
	int samples;
	float *data;
	int pos;
} audio;

static struct {
	int start, end;
} loop;

static void audioCallback(void *unused, float *samples, int nsamples) {
	(void)unused;
	for (int i = 0; i < nsamples; ++i) {
		//samples[i] = audio.data[audio.pos];
		samples[i * 2] = audio.data[audio.pos * 2];
		samples[i * 2 + 1] = audio.data[audio.pos * 2 + 1];
		audio.pos = (audio.pos + 1) % audio.samples;
		if (audio.pos >= loop.end)
			audio.pos = loop.start;
	}
}

static void resize(ATimeUs ts, unsigned int w, unsigned int h) {
	(void)ts;
	(void)w; (void)h;
}

#define SAMPLES_PER_TICK 5000

static void paint(ATimeUs ts, float dt) {
	(void)ts; (void)dt;
	video_paint((float)audio.pos / SAMPLES_PER_TICK);// ts / 1e6f);
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)ts; (void)down;
	switch (key) {
	case AK_Esc:
		audioClose();
		aAppTerminate(0);
		break;

	default:
		break;
	}
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;

	FILE *f = fopen("audio.raw", "rb");
	fseek(f, 0L, SEEK_END);
	audio.samples = ftell(f) / (sizeof(float) * 2);
	fseek(f, 0L, SEEK_SET);
	audio.data = new float[audio.samples * 2];
	fread(audio.data, sizeof(float) * 2, audio.samples, f);
	fclose(f);

	loop.start = 0;
	loop.end = audio.samples / SAMPLES_PER_TICK;

	loop.start += 128;
	loop.start += 128;
	//loop.start += 192;
	loop.start += 256;
	loop.start += 256 - 8;
	//loop.start += 512 - 64;
	//loop.start += 160;
	//loop.start += 192;
	//loop.start += 384-16;
	//loop.start -= 32;
	//loop.start += 256;
	//loop.start += 512;

	loop.start = 0;
	loop.end = loop.start + 128;

	loop.start *= SAMPLES_PER_TICK;
	loop.end *= SAMPLES_PER_TICK;

	audio.pos = loop.start;

	video_init(WIDTH, HEIGHT);
	audioOpen(44100, 2, nullptr, audioCallback, nullptr, nullptr);
}
