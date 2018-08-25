#include <stdio.h>

#include "video.h"
#include "atto/app.h"
#include "atto/platform.h"
//#define ATTO_UDIO_SAMPLERATE 44100
//#define ATTO_UDIO_H_IMPLEMENT
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#ifndef WIDTH
#define WIDTH 1280
#endif
#ifndef HEIGHT
#define HEIGHT 720
#endif

#define MSG(...) aAppDebugPrintf(__VA_ARGS__)

#include "4klang.h"

static struct {
	int samples;
	float *data;
	int pos;
} audio;

static struct {
	int paused;
	int start, end;
} loop;

static void audioCallback(void *unused, float *samples, int nsamples) {
	(void)unused;
	if (loop.paused) {
		memset(samples, 0, sizeof(*samples) * nsamples * 2);
		return;
	}

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

static struct {
	ATimeUs last_print;
	int frames;
} fpstat;

static void paint(ATimeUs ts, float dt) {
	const ATimeUs last_print_delta = ts - fpstat.last_print;
	if (last_print_delta > 1000000) {
		MSG("avg fps: %.1f", fpstat.frames * 1000000.f / last_print_delta);
		fpstat.frames = 0;
		fpstat.last_print = ts;
	}

	++fpstat.frames;

	(void)ts; (void)dt;
	video_paint((audio.pos + (loop.paused * rand() % SAMPLES_PER_TICK)) / (float)SAMPLES_PER_TICK);// ts / 1e6f);
}

const int pattern_length = 16;

static void timeShift(int ticks) {
	int next_pos = audio.pos + ticks * SAMPLES_PER_TICK;
	const int loop_length = loop.end - loop.start;
	while (next_pos < loop.start)
		next_pos += loop_length;
	while (next_pos > loop.end)
		next_pos -= loop_length;
	audio.pos = next_pos;
	MSG("pos = %d", next_pos / SAMPLES_PER_TICK);
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)ts;
	if (!down)
		return;

	switch (key) {
	case AK_Esc:
		audioClose();
		aAppTerminate(0);
		break;

	case AK_Left:
		timeShift(-pattern_length);
		break;
	case AK_Right:
		timeShift(pattern_length);
		break;
	case AK_Up:
		timeShift(8*pattern_length);
		break;
	case AK_Down:
		timeShift(-8*pattern_length);
		break;

	case AK_Space:
		loop.paused ^= 1;
		break;

	default:
		break;
	}
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;

	fpstat.last_print = 0;

	FILE *f = fopen("audio.raw", "rb");
	fseek(f, 0L, SEEK_END);
	audio.samples = ftell(f) / (sizeof(float) * 2);
	fseek(f, 0L, SEEK_SET);
	audio.data = new float[audio.samples * 2];
	fread(audio.data, sizeof(float) * 2, audio.samples, f);
	fclose(f);

	loop.start = 0;
	loop.end = audio.samples / SAMPLES_PER_TICK;

	loop.start *= SAMPLES_PER_TICK;
	loop.end *= SAMPLES_PER_TICK;

	audio.pos = loop.start;

	video_init(WIDTH, HEIGHT);
	audioOpen(44100, 2, nullptr, audioCallback, nullptr, nullptr);
}
