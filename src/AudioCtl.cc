#include "AudioCtl.h"

#ifndef ATTO_PLATFORM_RPI
#define AUDIO_IMPLEMENT
#include "aud_io.h"
#endif

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

#ifndef ATTO_PLATFORM_RPI
	if (!mute)
		audioOpen(settings.audio.samplerate, settings.audio.channels, nullptr, audioCallback, nullptr, nullptr);
#endif

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

	Timecode timecode() const noexcept {
		const float fpos = (float)pos_;
		return Timecode {
			fpos / (float)settings_.samples_per_row,
			fpos / (float)settings_.samplerate,
		};
	}

	if (mute && !loop.paused) {
		const int nsamples = dt * settings.audio.samplerate;
		loop.pos = (loop.pos + nsamples) % settings.audio.samples;
	}

void AudioCtl::AudioCtl(ProjectSettings settings) {
	loop.start = 0;
	loop.end = settings.audio.samples / settings.audio.samples_per_row;

	loop.start *= settings.audio.samples_per_row;
	loop.end *= settings.audio.samples_per_row;

	loop.pos = loop.start;

	MSG("float t = s / %f;", (float)settings.audio.samples_per_row * sizeof(float) * settings.audio.channels);

}

void AudioCtl::key(AKey key, bool pressed) {
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
}
