#include "AudioCtl.h"

#define AUDIO_IMPLEMENT
#include "aud_io.h"

#include "imgui.h"

AudioCtl::AudioCtl(const ProjectSettings& settings) noexcept
	: settings_(settings.audio)
	, end_(settings_.samples) {
	MSG("float t = s / %f;", (float)settings_.samples_per_row * sizeof(float) * settings_.channels);
	audioOpen(settings_.samplerate, settings_.channels, this, audioCallback, nullptr, nullptr);
}

bool AudioCtl::key(AKey key, bool pressed) noexcept {
	switch (key) {
	case AK_Left:
		timeShift(-settings_.pattern_length);
		break;
	case AK_Right:
		timeShift(settings_.pattern_length);
		break;
	case AK_Up:
		timeShift(4*settings_.pattern_length);
		break;
	case AK_Down:
		timeShift(-4*settings_.pattern_length);
		break;

	case AK_M:
		muted_ = !muted_;
		break;

	case AK_Space:
		paused_ ^= 1;
		break;

	case AK_Z:
		switch (set_) {
		case 0:
			start_ = ((pos_ / settings_.samples_per_row) / settings_.pattern_length) * settings_.samples_per_row * settings_.pattern_length;
			set_ = 1;
			break;
		case 1:
			end_ = (((pos_ / settings_.samples_per_row) + (settings_.pattern_length-1)) / settings_.pattern_length) * settings_.samples_per_row * settings_.pattern_length;
			set_ = 2;
			break;
		case 2:
			start_ = 0;
			end_ = settings_.samples;
			set_ = 0;
		}
		break;
	default:
		return false;
	}

	return true;
}

void AudioCtl::timeShift(int rows) noexcept {
	int next_pos = pos_ + rows * settings_.samples_per_row;
	const int loop_length = end_ - start_;
	while (next_pos < start_)
		next_pos += loop_length;
	while (next_pos > end_)
		next_pos -= loop_length;
	pos_ = next_pos;
	MSG("pos = %d", next_pos / settings_.samples_per_row);
}

Timecode AudioCtl::timecode(ATimeUs ts, float dt) noexcept {
	// if (muted_ && !paused_) {
	// 	const int nsamples = dt * settings_.samplerate;
	// 	pos_ = (pos_ + nsamples) % settings_.samples;
	// }

	const float fpos = (float)pos_;
	return Timecode {
		fpos / (float)settings_.samples_per_row,
		fpos / (float)settings_.samplerate,
	};
}

void AudioCtl::audioCallback(void *actl, float *samples, int nsamples) noexcept {
	((AudioCtl*)actl)->audioCallback(samples, nsamples);
}

void AudioCtl::audioCallback(float *samples, int nsamples) noexcept {
	const bool paused = paused_;
	if (paused || !settings_.data || muted_) {
		memset(samples, 0, sizeof(*samples) * nsamples * 2);
		if (!paused)
			pos_ = (pos_ + nsamples) % settings_.samples;
		return;
	}

	for (int i = 0; i < nsamples; ++i) {
		samples[i * 2] = settings_.data[pos_ * 2];
		samples[i * 2 + 1] = settings_.data[pos_ * 2 + 1];
		pos_ = (pos_ + 1) % settings_.samples;

		if (set_ == 2)
			if (pos_ >= end_)
				pos_ = start_;
	}
}

void AudioCtl::paint() noexcept {
	if (ImGui::Begin("AudioCtl")) {
		ImGui::Checkbox("Pause", &paused_);

		const Timecode tc = timecode(0,0);
		ImGui::LabelText("Pos", "%.3f (%.3fsec)", tc.row, tc.sec);

		if (settings_.data) {
			if (ImGui::BeginChild("timeline", ImVec2(-1, 0.f), false, ImGuiWindowFlags_HorizontalScrollbar)) {
				ImGui::SetNextItemWidth(-1);
				ImGui::PlotLines("", settings_.data, settings_.samples, 0, "", -1.f, 1.f, ImVec2(settings_.samples / 4410.f, 100));
			}
			ImGui::EndChild();
		}
	}

	ImGui::End();
}
