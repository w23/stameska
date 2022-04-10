#include "Root.hh"

#include "Rocket.h"
#include "AutomationBasic.h"
#include "GuiScope.h"
#include "Variables.h"

#include "OpenGL.h"

#include "imgui.h"
#include "ui.h"

std::string RootNode::root_node_class_name_ = "RootNode";

RootNode::RootNode(const fs::path &root, const ProjectSettings &settings)
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

	ui_init();
}

RootNode::~RootNode() {}

void RootNode::resize(int w, int h) {
	ui_resize();
	video_.resize(w,h);
}

void RootNode::paint(ATimeUs ts, float dt) {
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

	{
		ui_begin(dt, timecode.row, timecode.sec);

		if (ImGui::Begin("NodeTree")) {
			visitChildren(nodeTreeFunc);
		}
		ImGui::End();

		if (automation_)
			automation_->paint();
		audio_.paint();
		ui_end();
	}
}

void RootNode::key(ATimeUs ts, AKey key, int down) {
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

void RootNode::pointer(ATimeUs ts, int dx, int dy, unsigned int dbtn) {
	(void)ts; (void)dx; (void)dy; (void)dbtn;
	ui_mouse();
}

void RootNode::visitChildren(const std::function<void(INode*)>& visitor) noexcept {
	visitor(&video_);
}

void RootNode::nodeTreeFunc(INode* node) {
	if (ImGui::TreeNode(node->getName().c_str())) {
		node->doUi();
		node->visitChildren(nodeTreeFunc);
		ImGui::TreePop();
	}
}
