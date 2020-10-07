#include "ProjectSettings.h"
#include "Rocket.h"
#include "AutomationBasic.h"
#include "GuiScope.h"
#include "Variables.h"
#include "AudioCtl.h"
#include "VideoNode.h"

#include "ui.h"
#include "utils.h"
#include "filesystem.h"
#include "atto/app.h"
#include "atto/platform.h"
#include "OpenGL.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>


class RootNode : public INode {
public:
	RootNode(const fs::path &root, const ProjectSettings &settings)
		: INode("RootNode", "/")
		, video_(root, settings.video.config_filename)
		, audio_(settings)
	{
		switch (settings.automation.type) {
			case ProjectSettings::Automation::Type::Rocket:
				automation_.reset(new Rocket(
					[](int pause) {
						// FIXME loop.paused = pause;
					},
					[](int row) {
						// FIXME loop.pos = row * g_settings.audio.samples_per_row;
					},
					[]() {
						// FIXME return !loop.paused.load();
						return false;
					}
				));
				break;
			case ProjectSettings::Automation::Type::Basic:
				automation_.reset(new AutomationBasic((root/settings.automation.filename).string()));
				break;
			case ProjectSettings::Automation::Type::Gui:
				automation_.reset(new GuiScope());
				break;
			case ProjectSettings::Automation::Type::None:
				MSG("Not using any automation");
				break;
		}
	}
    void resize(int w, int h) {
		ui_resize();
		video_.resize(w,h);
	}
	
    void paint(ATimeUs ts, float dt) {
		const Timecode timecode = audio_.timecode(ts, dt);

		{
			const ATimeUs last_print_delta = ts - fpstat.last_print;
			if (last_print_delta > 1000000) {
				MSG("row=%f, avg fps: %.1f %.2f", timecode.row, fpstat.frames * 1000000.f / last_print_delta, dt*1e3f);
				fpstat.frames = 0;
				fpstat.last_print = ts;
			}

			++fpstat.frames;
		}

		if (automation_)
			automation_->update(timecode.row);

		DummyScope dummy_scope;
		IScope *dummy = &dummy_scope;

		video_.paint(dt, timecode, automation_ ? *automation_.get() : *dummy);

		ui_begin(dt, timecode.row, timecode.sec);
		video_.paintUi();
		if (automation_)
			automation_->paint();
		audio_.paint();
		ui_end();
	}

	void key(ATimeUs ts, AKey key, int down) {
		if (audio_.key(key, down))
			return;

		if (!down)
			return;

		switch (key) {
		case AK_Esc:
			//audioClose();
			aAppTerminate(0);
			break;

	/*
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
			video_export(g_settings.exports, *automation.get());
			break;
			*/

		default:
			MSG("Unknown key %d", key);
			break;
		}
	}

	virtual void visitChildren(std::function<void(INode*)> visitor) noexcept override {
		visitor(&video_);
	}

private:
	static std::string root_node_class_name_;
	std::unique_ptr<IAutomation> automation_;
	AudioCtl audio_;
	VideoNode video_;

	struct {
		ATimeUs last_print = 0;
		int frames = 0;
	} fpstat;
};

std::string RootNode::root_node_class_name_ = "RootNode";

static std::unique_ptr<RootNode> g_root;
static ProjectSettings g_settings;

static void resize(ATimeUs ts, unsigned int w, unsigned int h) {
	(void)ts;
	(void)w; (void)h;
	g_root->resize(a_app_state->width, a_app_state->height);
}

static void paint(ATimeUs ts, float dt) {
	g_root->paint(ts, dt);
}

static void key(ATimeUs ts, AKey key, int down) {
	(void)ts;
	g_root->key(ts, key, down);
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

	const char *settings_filename = nullptr;

	if (a_app_state->argc < 2) {
		MSG("Usage: %s project.yaml", a_app_state->argv[0]);
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

		g_settings = std::move(settings_result).value();
	}

	g_root.reset(new RootNode(fs::path(settings_filename).remove_filename(), g_settings));

	ui_init();
}
