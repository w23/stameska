#include "ProjectSettings.h"
#include "Rocket.h"
#include "AutomationBasic.h"
#include "GuiScope.h"
#include "Variables.h"
#include "AudioCtl.h"

#include "ui.h"
#include "video.h"
#include "utils.h"
#include "filesystem.h"
#include "atto/app.h"
#include "atto/platform.h"
#include "OpenGL.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>

static std::unique_ptr<IAutomation> automation;
static ProjectSettings settings;
static std::unique_ptr<AudioCtl> g_audio_ctl;

static struct { int w, h; } canvas_sizes[] = {
	{1920, 1080},
	{1280, 720},
	{960, 540},
	{640, 360},
	{320, 180},
};
static int canvas_size_cursor = 0;

static void resize(ATimeUs ts, unsigned int w, unsigned int h) {
	(void)ts;
	(void)w; (void)h;
	video_preview_resize(a_app_state->width, a_app_state->height);
	ui_resize();
}

static struct {
	ATimeUs last_print;
	int frames;
} fpstat;

static void paint(ATimeUs ts, float dt) {
	const Timecode timecode = g_audio_ctl->timecode(ts, dt);

	{
		const ATimeUs last_print_delta = ts - fpstat.last_print;
		if (last_print_delta > 1000000) {
			MSG("row=%f, avg fps: %.1f %.2f", timecode.row, fpstat.frames * 1000000.f / last_print_delta, dt*1e3f);
			fpstat.frames = 0;
			fpstat.last_print = ts;
		}

		++fpstat.frames;
	}

	if (automation)
		automation->update(timecode.row);

	DummyScope dummy_scope;
	IScope *dummy = &dummy_scope;

	video_paint(timecode.row, dt, automation ? *automation.get() : *dummy);
	ui_begin(dt, timecode.row, timecode.sec);
	if (automation)
		automation->paint();
	ui_end();
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)ts;

	if (g_audio_ctl->key(key, down))
		return;

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

	case AK_E:
		automation->save();
		video_export(settings.exports, *automation.get());
		break;

	default:
		MSG("Unknown key %d", key);
		break;
	}
}

static void pointer(ATimeUs, int dx, int dy, unsigned int dbtn) {
	(void)dx; (void)dy; (void)dbtn;
	ui_mouse();
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;
	proctable->pointer = pointer;

	fpstat.last_print = 0;
	const char *settings_filename = nullptr;

	if (a_app_state->argc < 2) {
		MSG("Usage: %s <--mute> project.yaml", a_app_state->argv[0]);
		aAppTerminate(1);
	}

	for (int i = 1; i < a_app_state->argc; ++i) {
		const char *arg = a_app_state->argv[i];
		// if (strcmp(arg,"--mute") == 0) g_audio_ctl.reset(new AudioCtl());
		// else
		settings_filename = arg;
	}

	{
		auto settings_result = ProjectSettings::readFromFile(settings_filename);
		if (!settings_result) {
			MSG("Error reading project file %s: %s", settings_filename, settings_result.error().c_str());
			aAppTerminate(2);
		}

		settings = std::move(settings_result).value();
	}

	fs::path project_root = fs::path(settings_filename).remove_filename();

	switch (settings.automation.type) {
		case ProjectSettings::Automation::Type::Rocket:
			automation.reset(new Rocket(
				[](int pause) {
					// FIXME loop.paused = pause;
				},
				[](int row) {
					// FIXME loop.pos = row * settings.audio.samples_per_row;
				},
				[]() {
					// FIXME return !loop.paused.load();
					return false;
				}
			));
			break;
		case ProjectSettings::Automation::Type::Basic:
			automation.reset(new AutomationBasic((project_root/settings.automation.filename).string()));
			break;
		case ProjectSettings::Automation::Type::Gui:
			automation.reset(new GuiScope());
			break;
		case ProjectSettings::Automation::Type::None:
			MSG("Not using any automation");
			break;
	}

	video_init(std::move(project_root), settings.video.config_filename);
	MSG("Set resolution: %dx%d",
		canvas_sizes[canvas_size_cursor].w,
		canvas_sizes[canvas_size_cursor].h);
	video_canvas_resize(
		canvas_sizes[canvas_size_cursor].w,
		canvas_sizes[canvas_size_cursor].h);

	ui_init();

	g_audio_ctl.reset(new AudioCtl(settings));
}
