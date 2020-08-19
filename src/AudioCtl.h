#pragma once

#include "ProjectSettings.h"
#include "atto/app.h"

#include <atomic>

struct Timecode {
	float row, sec;
};

class AudioCtl {
public:
	AudioCtl(/*muted*/) noexcept;
	AudioCtl(const ProjectSettings& settings) noexcept;
	~AudioCtl() {}

	Timecode timecode(ATimeUs ts, float dt) const noexcept;

	bool key(AKey key, bool pressed) noexcept;

private:
	const float rows_per_sample_ = 0;
	const float seconds_per_sample_ = 0;
	const int samplerate_ = 0;
	const int channels_ = 0;
	const float* const samples_ = nullptr;

	bool muted_ = true;

	std::atomic<int> pos_ = {0};
	std::atomic<int> paused_ = {0};
	int start_ = 0, end_ = 0;
	int set_ = 0;
};
