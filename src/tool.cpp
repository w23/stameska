#include "Timeline.h"

#include "video.h"
#include "utils.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"
#include "atto/app.h"
#include "atto/platform.h"

#include "json.hpp"
#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>
#include <fstream>

using json = nlohmann::json;

#define SAMPLE_TYPE float

static std::unique_ptr<Timeline> timeline;

static struct {
	int samples_per_tick;
	int samples;
	float *data;
	std::atomic<int> pos;
} audio;

static struct {
	std::atomic<int> paused;
	int start, end;
	int set;
} loop;

static void audioCallback(void *unused, float *samples, int nsamples) {
	(void)unused;
	if (loop.paused || !audio.data) {
		memset(samples, 0, sizeof(*samples) * nsamples * 2);
		if (!loop.paused)
			audio.pos = (audio.pos + nsamples) % audio.samples;
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
	(void)ts; (void)dt;

	const ATimeUs last_print_delta = ts - fpstat.last_print;
	if (last_print_delta > 1000000) {
		MSG("avg fps: %.1f", fpstat.frames * 1000000.f / last_print_delta);
		fpstat.frames = 0;
		fpstat.last_print = ts;
	}

	++fpstat.frames;

	const float time_row = (float)audio.pos / audio.samples_per_tick;
	timeline->update(time_row);

	video_paint(time_row, *timeline.get());
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

struct Arg {
	const char *arg;
	const char **ptr;
	const char *info;
};

static int parseArgs(int argc, const char * const *argv, const struct Arg *args, int nargs) {
	int i = 1;
	for (; i < argc; ++i) {
		int j = 0;
		for (; j < nargs; ++j) {
			const struct Arg *a = args + j;
			if (strcmp(argv[i], a->arg) == 0) {
				++i;
				if (i >= argc) {
					MSG("Error: option %s requires an argument", a->arg);
					return -1;
				}
				*a->ptr = argv[i];
				break;
			}
		}
		if (j == nargs)
			return -i;
	}
	return i - 1;
}

json readJson(const char *filename) {
	try {
		std::ifstream i(filename, std::ifstream::in);
		json j;
		i >> j;
		return j;
	} catch (const std::exception& e) {
		MSG("Error reading file %s: %s", filename, e.what());
		return json();
	}
}

bool loadAudio(const char *filename) {
	MSG("Reading audio settings from file %s", filename);
	json j = readJson(filename);
	if (j.is_null())
		j = json::object();

	FILE *f = nullptr;
	audio.samples_per_tick = j.value("samples_per_tick", 4096);
	const std::string& raw_file = j.value("raw_file", "");
	f = fopen(raw_file.c_str(), "rb");

	if (f) {
		fseek(f, 0L, SEEK_END);
		audio.samples = ftell(f) / (sizeof(float) * 2);
		fseek(f, 0L, SEEK_SET);
		audio.data = new float[audio.samples * 2];
		audio.samples = fread(audio.data, sizeof(float) * 2, audio.samples, f);
		fclose(f);
	} else {
		MSG("No audio file given, continuing in silence");
		audio.samples = 44100 * j.value("duration_sec", 120);
		audio.data = NULL;
	}

	loop.start = 0;
	loop.end = audio.samples / audio.samples_per_tick;

	loop.start *= audio.samples_per_tick;
	loop.end *= audio.samples_per_tick;

	audio.pos = loop.start;

	MSG("float t = s / %f;", (float)audio.samples_per_tick * sizeof(SAMPLE_TYPE) * 2);
	return true;
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;

	loop.set = 0;
	fpstat.last_print = 0;

	const char *audio_config = NULL;
	const char *video_config = NULL;

	static const Arg args[] = {
		{"-a", &audio_config, "Audio/sync configuration file" },
		{"-v", &video_config, "Video configuration file" },
	};

	const int parsed = parseArgs(a_app_state->argc, a_app_state->argv, args, (int)COUNTOF(args));
	if (parsed < 0 || !audio_config || !video_config) {
		MSG("Usage: %s", a_app_state->argv[0]);
		for (int i = 0; i < (int)COUNTOF(args); ++i)
			MSG("\t%s\t-\t%s", args[i].arg, args[i].info);
		aAppTerminate(1);
	}

	if (!loadAudio(audio_config)) {
		aAppTerminate(2);
	}

	MSG("using shader file %s", video_config);
	video_init(1920, 1080, video_config);

	timeline.reset(new Timeline(
		[](int pause) {
			loop.paused = pause;
		},
		[](int row) {
			audio.pos = row * audio.samples_per_tick;
		},
		[]() {
			return loop.paused.load();
		}
	));
	audioOpen(44100, 2, nullptr, audioCallback, nullptr, nullptr);
}
