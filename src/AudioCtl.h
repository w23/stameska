#pragma once

#include "ProjectSettings.h"
#include "atto/app.h"

#include <atomic>

struct Timecode {
	float row, sec;
};

class AudioCtl {
public:
	AudioCtl(const ProjectSettings& settings) noexcept;
	~AudioCtl() {}

	Timecode timecode(ATimeUs ts, float dt) noexcept;

	bool key(AKey key, bool pressed) noexcept;

	void paint() noexcept;

	void pause(int pause) { paused_ = !!pause; }
	void setTimeRow(int row) { pos_ = settings_.samples_per_row * row; }
	bool paused() { return paused_; }

private:
	const ProjectSettings::Audio& settings_;

	bool muted_ = false;

	std::atomic<int> pos_ = {0};
	bool paused_ = false;
	int start_ = 0, end_ = 0;
	int set_ = 0;

	static void audioCallback(void *unused, float *samples, int nsamples) noexcept;
	void audioCallback(float *samples, int nsamples) noexcept;
	void timeShift(int rows) noexcept;
};
