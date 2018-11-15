#include <stdio.h>

#include "video.h"
#include "atto/app.h"
#include "atto/platform.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#define MSG(...) aAppDebugPrintf(__VA_ARGS__)

#define SAMPLE_TYPE float

static struct {
	int samples_per_tick;
	int samples;
	float *data;
	int pos;
} audio;

static struct {
	int paused;
	int start, end;
	int set;
} loop;

static void audioCallback(void *unused, float *samples, int nsamples) {
	(void)unused;
	if (loop.paused) {
		memset(samples, 0, sizeof(*samples) * nsamples * 2);
		return;
	}

	for (int i = 0; i < nsamples; ++i) {
		samples[i * 2] = audio.data[audio.pos * 2];
		samples[i * 2 + 1] = audio.data[audio.pos * 2 + 1];
		audio.pos = (audio.pos + 1) % audio.samples;

		if (loop.set == 2)
			if (audio.pos >= loop.end)
				audio.pos = loop.start;
	}
}

static void resize(ATimeUs ts, unsigned int w, unsigned int h) {
	(void)ts;
	(void)w; (void)h;
	video_tool_resize(a_app_state->width, a_app_state->height);
}

static struct {
	ATimeUs last_print;
	int frames;
} fpstat;

static void paint(ATimeUs ts, float dt) {
	(void)dt;
	const ATimeUs last_print_delta = ts - fpstat.last_print;
	if (last_print_delta > 1000000) {
		MSG("avg fps: %.1f", fpstat.frames * 1000000.f / last_print_delta);
		fpstat.frames = 0;
		fpstat.last_print = ts;
	}

	++fpstat.frames;

	(void)ts; (void)dt;
	const int byte_pos = audio.pos * sizeof(SAMPLE_TYPE) * 2;
	//video_paint((audio.pos /* + (loop.paused * rand() % audio.samples_per_tick / 2)*/) / (float)audio.samples_per_tick);// ts / 1e6f);
	video_paint(byte_pos + (loop.paused * rand() % audio.samples_per_tick) * sizeof(SAMPLE_TYPE) * 2);
	//video_paint(byte_pos);
}

const int pattern_length = 64;

static void timeShift(int ticks) {
	int next_pos = audio.pos + ticks * audio.samples_per_tick;
	const int loop_length = loop.end - loop.start;
	while (next_pos < loop.start)
		next_pos += loop_length;
	while (next_pos > loop.end)
		next_pos -= loop_length;
	audio.pos = next_pos;
	MSG("pos = %d", next_pos / audio.samples_per_tick);
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)ts;

	if (!down)
		return;

	switch (key) {
	case AK_Esc:
		//audioClose();
		aAppTerminate(0);
		break;

	case AK_Left:
		timeShift(-pattern_length);
		break;
	case AK_Right:
		timeShift(pattern_length);
		break;
	case AK_Up:
		timeShift(4*pattern_length);
		break;
	case AK_Down:
		timeShift(-4*pattern_length);
		break;

	case AK_Space:
		loop.paused ^= 1;
		break;

	case AK_Z:
		switch (loop.set) {
		case 0:
			loop.start = ((audio.pos / audio.samples_per_tick) / 64) * audio.samples_per_tick * 64;
			loop.set = 1;
			break;
		case 1:
			loop.end = (((audio.pos / audio.samples_per_tick) + 63) / 64) * audio.samples_per_tick * 64;
			loop.set = 2;
			break;
		case 2:
			loop.start = 0;
			loop.end = audio.samples;
			loop.set = 0;
		}
		break;

	default:
		MSG("Unknown key %d", key);
		break;
	}
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;

	loop.set = 0;
	fpstat.last_print = 0;

	// TODO args
	// FIXME args
	audio.samples_per_tick = 5000;

	FILE *f = fopen("audio.raw", "rb");
	if (f) {
		fseek(f, 0L, SEEK_END);
		audio.samples = ftell(f) / (sizeof(float) * 2);
		fseek(f, 0L, SEEK_SET);
		audio.data = new float[audio.samples * 2];
		fread(audio.data, sizeof(float) * 2, audio.samples, f);
		fclose(f);
	} else {
		audio.samples = 44100 * 120;
		audio.data = NULL;
	}

	loop.start = 0;
	loop.end = audio.samples / audio.samples_per_tick;

	loop.start *= audio.samples_per_tick;
	loop.end *= audio.samples_per_tick;

	audio.pos = loop.start;

	MSG("float t = s / %f;", (float)audio.samples_per_tick * sizeof(SAMPLE_TYPE) * 2);
	video_init(WIDTH, HEIGHT);
	audioOpen(44100, 2, nullptr, audioCallback, nullptr, nullptr);
}
