#include "RootNode.hh"
#include "ProjectSettings.h"

#include "utils.h"
#include "filesystem.h"
#include "atto/app.h"
#include "atto/platform.h"

#include <stdio.h>
#include <string.h>
#include <memory>
#include <atomic>

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
	g_root->key(ts, key, down);
}

static void pointer(ATimeUs ts, int dx, int dy, unsigned int dbtn) {
	g_root->pointer(ts, dx, dy, dbtn);
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
}
