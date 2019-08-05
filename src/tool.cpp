#include "ProjectSettings.h"
#include "Timeline.h"

#include "video.h"
#include "utils.h"
#ifndef ATTO_PLATFORM_RPI
#define AUDIO_IMPLEMENT
#include "aud_io.h"
#endif
#include "atto/app.h"
#include "atto/platform.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>

static std::unique_ptr<Timeline> timeline;
static ProjectSettings settings;

static struct {
	std::atomic<int> pos;
	std::atomic<int> paused;
	int start, end;
	int set;
} loop;

static struct { int w, h; } canvas_sizes[] = {
	{1920, 1080},
	{1280, 720},
	{960, 540},
	{640, 360}
};
static int canvas_size_cursor = 0;

#ifndef ATTO_PLATFORM_RPI
static void audioCallback(void *unused, float *samples, int nsamples) {
	(void)unused;
	if (loop.paused || !settings.audio.data) {
		memset(samples, 0, sizeof(*samples) * nsamples * 2);
		if (!loop.paused)
			loop.pos = (loop.pos + nsamples) % settings.audio.samples;
		return;
	}

	for (int i = 0; i < nsamples; ++i) {
		samples[i * 2] = settings.audio.data[loop.pos * 2];
		samples[i * 2 + 1] = settings.audio.data[loop.pos * 2 + 1];
		loop.pos = (loop.pos + 1) % settings.audio.samples;

		if (loop.set == 2)
			if (loop.pos >= loop.end)
				loop.pos = loop.start;
	}
}
#endif

static void resize(ATimeUs ts, unsigned int w, unsigned int h) {
	(void)ts;
	(void)w; (void)h;
	video_preview_resize(a_app_state->width, a_app_state->height);
}

static struct {
	ATimeUs last_print;
	int frames;
} fpstat;

static void paint(ATimeUs ts, float dt) {
	const ATimeUs last_print_delta = ts - fpstat.last_print;
	if (last_print_delta > 1000000) {
		MSG("avg fps: %.1f %.2f", fpstat.frames * 1000000.f / last_print_delta, dt*1e3f);
		fpstat.frames = 0;
		fpstat.last_print = ts;
	}

	++fpstat.frames;

	const float time_row = (float)loop.pos / settings.audio.samples_per_row;
	timeline->update(time_row);

	video_paint(time_row, dt, *timeline.get());
}

const int pattern_length = 16;

static void timeShift(int rows) {
	int next_pos = loop.pos + rows * settings.audio.samples_per_row;
	const int loop_length = loop.end - loop.start;
	while (next_pos < loop.start)
		next_pos += loop_length;
	while (next_pos > loop.end)
		next_pos -= loop_length;
	loop.pos = next_pos;
	MSG("pos = %d", next_pos / settings.audio.samples_per_row);
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

	case AK_Plus:
	case AK_Equal:
	case AK_KeypadPlus:
		if (canvas_size_cursor > 0) {
			--canvas_size_cursor;
			MSG("Change resolution: %dx%d",
				canvas_sizes[canvas_size_cursor].w,
				canvas_sizes[canvas_size_cursor].h);
			video_canvas_resize(
				canvas_sizes[canvas_size_cursor].w,
				canvas_sizes[canvas_size_cursor].h);
		}
		break;
	case AK_Minus:
	case AK_KeypadMinus:
		if (canvas_size_cursor < (int)(COUNTOF(canvas_sizes) - 1)) {
			++canvas_size_cursor;
			MSG("Change resolution: %dx%d",
				canvas_sizes[canvas_size_cursor].w,
				canvas_sizes[canvas_size_cursor].h);
			video_canvas_resize(
				canvas_sizes[canvas_size_cursor].w,
				canvas_sizes[canvas_size_cursor].h);
		}
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

	case AK_E:
		timeline->save();
		video_export(settings.exports);
		break;

	case AK_Space:
		loop.paused ^= 1;
		break;

	case AK_Z:
		switch (loop.set) {
		case 0:
			loop.start = ((loop.pos / settings.audio.samples_per_row) / 64) * settings.audio.samples_per_row * 64;
			loop.set = 1;
			break;
		case 1:
			loop.end = (((loop.pos / settings.audio.samples_per_row) + 63) / 64) * settings.audio.samples_per_row * 64;
			loop.set = 2;
			break;
		case 2:
			loop.start = 0;
			loop.end = settings.audio.samples;
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

	if (a_app_state->argc != 2) {
		MSG("Usage: %s project.json", a_app_state->argv[0]);
		aAppTerminate(1);
	}

	{
		const char *settings_filename = a_app_state->argv[1];
		auto settings_result = ProjectSettings::readFromFile(settings_filename);
		if (!settings_result) {
			MSG("Error reading project file %s: %s", settings_filename, settings_result.error().c_str());
			aAppTerminate(2);
		}

		settings = std::move(settings_result).value();
	}

	loop.start = 0;
	loop.end = settings.audio.samples / settings.audio.samples_per_row;

	loop.start *= settings.audio.samples_per_row;
	loop.end *= settings.audio.samples_per_row;

	loop.pos = loop.start;

	MSG("float t = s / %f;", (float)settings.audio.samples_per_row * sizeof(float) * settings.audio.channels);

	video_init(settings.video.config_filename.c_str());
	MSG("Set resolution: %dx%d",
		canvas_sizes[canvas_size_cursor].w,
		canvas_sizes[canvas_size_cursor].h);
	video_canvas_resize(
		canvas_sizes[canvas_size_cursor].w,
		canvas_sizes[canvas_size_cursor].h);

	timeline.reset(new Timeline(
		[](int pause) {
			loop.paused = pause;
		},
		[](int row) {
			loop.pos = row * settings.audio.samples_per_row;
		},
		[]() {
			return !loop.paused.load();
		}
	));
#ifndef ATTO_PLATFORM_RPI
	audioOpen(settings.audio.samplerate, settings.audio.channels, nullptr, audioCallback, nullptr, nullptr);
#endif
}
