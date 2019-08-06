#include "Rocket.h"
#include "utils.h"
#include "rocket/lib/sync.h"
#include <math.h>
#include <cassert>

Rocket::Rocket(PauseCallback&& pause, SetRowCallback&& setRow, IsPlayingCallback&& isPlaying)
	: pauseCallback(pause)
	, setRowCallback(setRow)
	, isPlayingCallback(isPlaying)
	, rocket_(sync_create_device("sync"))
{
	if (!rocket_)
		CRASH("Cannot create rocket");

	sync_tcp_connect(rocket_.get(), "localhost", SYNC_DEFAULT_PORT);
}

Rocket::~Rocket() {
	sync_save_tracks(rocket_.get());
}

void Rocket::update(float row) {
	struct sync_cb callbacks = { pause, set_row, is_playing };
	if (sync_update(rocket_.get(), (int)floorf(row), &callbacks, this))
		sync_tcp_connect(rocket_.get(), "localhost", SYNC_DEFAULT_PORT);

	row_ = row;
}

static const char* component_suffix[4] = { ".x", ".y", ".z", ".w" };

Value Rocket::getValue(const std::string& name, int comps) {
	assert(comps > 0);
	assert(comps <= 4);

	Value v = { 0, 0, 0, 0 };
	const auto it = vars_.find(name);
	if (it != vars_.end()) {
		for (int i = 0; i < comps; ++i)
			(&v.x)[i] = sync_get_val(it->second.component[i].track, row_);
		if (it->second.components != comps) {
			if (it->second.components < comps) {
				for (int i = it->second.components; i < comps; ++i) {
					it->second.component[i].name = name + component_suffix[i];
					it->second.component[i].track = sync_get_track(rocket_.get(), it->second.component[i].name.c_str());
				}
			} else if (comps == 1) {
				it->second.component[0].name = name;
				it->second.component[0].track = sync_get_track(rocket_.get(), it->second.component[0].name.c_str());
			}
			it->second.components = comps;
		}

		return v;
	}

	Variable var;
	var.components = comps;
	if (comps > 1) {
		for (int i = 0; i < comps; ++i) {
			var.component[i].name = name + component_suffix[i];
			var.component[i].track = sync_get_track(rocket_.get(), var.component[i].name.c_str());
		}
	} else {
		var.component[0].name = name;
		var.component[0].track = sync_get_track(rocket_.get(), var.component[0].name.c_str());
	}

	vars_[name] = std::move(var);
	return v;
}

void Rocket::save() const {
	sync_save_tracks(rocket_.get());
}

void Rocket::pause(void *t, int p) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	timeline->pauseCallback(p);
}

void Rocket::set_row(void *t, int r) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	timeline->setRowCallback(r);
}

int Rocket::is_playing(void *t) {
	Rocket *timeline = reinterpret_cast<Rocket*>(t);
	return timeline->isPlayingCallback();
}
