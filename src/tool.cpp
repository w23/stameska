#include "ProjectSettings.h"
#include "Rocket.h"
#include "AutomationBasic.h"
#include "Variables.h"

#include "video.h"
#include "utils.h"
#include "filesystem.h"
#ifndef ATTO_PLATFORM_RPI
#define AUDIO_IMPLEMENT
#include "aud_io.h"
#endif
#include "atto/app.h"
#include "atto/platform.h"
#include "OpenGL.h"

#include "imgui.h"
#include "examples/imgui_impl_opengl2.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>

static std::unique_ptr<IAutomation> automation;
static ProjectSettings settings;

static struct {
	std::atomic<int> pos;
	std::atomic<int> paused;
	int start, end;
	int set;
} loop;

bool mute = false;

static struct { int w, h; } canvas_sizes[] = {
	{1920, 1080},
	{1280, 720},
	{960, 540},
	{640, 360},
	{320, 180},
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

  ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)a_app_state->width, (float)a_app_state->height);
  io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
}

static struct {
	ATimeUs last_print;
	int frames;
} fpstat;

static void paint(ATimeUs ts, float dt) {
	const float time_row = (float)loop.pos / settings.audio.samples_per_row;
	const ATimeUs last_print_delta = ts - fpstat.last_print;
	if (last_print_delta > 1000000) {
		MSG("row=%f, avg fps: %.1f %.2f", time_row, fpstat.frames * 1000000.f / last_print_delta, dt*1e3f);
		fpstat.frames = 0;
		fpstat.last_print = ts;
	}

	if (mute && !loop.paused) {
		const int nsamples = dt * settings.audio.samplerate;
		loop.pos = (loop.pos + nsamples) % settings.audio.samples;
	}

	++fpstat.frames;

	if (automation)
		automation->update(time_row);

	DummyScope dummy_scope;
	IScope *dummy = &dummy_scope;

	video_paint(time_row, dt, automation ? *automation.get() : *dummy);

	// if (ImGui::Button("Save"))
	// 	    MySaveFunction();
	// ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
	// ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	{
		ImGui_ImplOpenGL2_NewFrame();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = dt;
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		ImGui::NewFrame();
	}
	ImGui::Text("Hello, world %d", 123);
	bool show_demo_window = true;
  ImGui::ShowDemoWindow(&show_demo_window);
  ImGui::Render();
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
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
		automation->save();
		video_export(settings.exports, *automation.get());
		break;

	case AK_Space:
		loop.paused ^= 1;
		break;

	case AK_Z:
		switch (loop.set) {
		case 0:
			loop.start = ((loop.pos / settings.audio.samples_per_row) / pattern_length) * settings.audio.samples_per_row * pattern_length;
			loop.set = 1;
			break;
		case 1:
			loop.end = (((loop.pos / settings.audio.samples_per_row) + (pattern_length-1)) / pattern_length) * settings.audio.samples_per_row * pattern_length;
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

static void pointer(ATimeUs, int dx, int dy, unsigned int dbtn) {
	const unsigned int btn = a_app_state->pointer.buttons;
	const int x = a_app_state->pointer.x;
	const int y = a_app_state->pointer.y;
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	io.MouseDown[0] = !!(btn & AB_Left);
	io.MouseDown[1] = !!(btn & AB_Right);
	io.MouseDown[2] = !!(btn & AB_Middle);
}

void attoAppInit(struct AAppProctable *proctable) {
	proctable->resize = resize;
	proctable->paint = paint;
	proctable->key = key;
	proctable->pointer = pointer;

	loop.set = 0;
	fpstat.last_print = 0;
	const char *settings_filename = nullptr;

	if (a_app_state->argc < 2) {
		MSG("Usage: %s <--mute> project.yaml", a_app_state->argv[0]);
		aAppTerminate(1);
	}

	for (int i = 1; i < a_app_state->argc; ++i) {
		const char *arg = a_app_state->argv[i];
		if (strcmp(arg,"--mute") == 0) mute = true;
		else settings_filename = arg;
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

	loop.start = 0;
	loop.end = settings.audio.samples / settings.audio.samples_per_row;

	loop.start *= settings.audio.samples_per_row;
	loop.end *= settings.audio.samples_per_row;

	loop.pos = loop.start;

	MSG("float t = s / %f;", (float)settings.audio.samples_per_row * sizeof(float) * settings.audio.channels);

	switch (settings.automation.type) {
		case ProjectSettings::Automation::Type::Rocket:
			automation.reset(new Rocket(
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
			break;
		case ProjectSettings::Automation::Type::Basic:
			automation.reset(new AutomationBasic((project_root/settings.automation.filename).string()));
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

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
	io.BackendPlatformName = "atto";
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  ImGui_ImplOpenGL2_Init();

#ifndef ATTO_PLATFORM_RPI
	if (!mute)
		audioOpen(settings.audio.samplerate, settings.audio.channels, nullptr, audioCallback, nullptr, nullptr);
#endif
}
