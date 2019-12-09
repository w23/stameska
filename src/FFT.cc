// This file is mostly taken as-is from Bonzomatic by Gargaj et al.
#include "utils.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <kiss_fft.h>
#include <tools/kiss_fftr.h>

#include <stdio.h>
#include <memory.h>

#include "FFT.h"

constexpr int ring_buffer_len = FFT::width * 4;

static struct {
	kiss_fftr_cfg fftcfg;
	ma_context context;
	ma_device captureDevice;
	int audio_ring_buffer_cursor = 0;
	float audio_ring_buffer[ring_buffer_len];
	float fft_buffer_in[FFT::width*2];
	float fft[FFT::width];
	float fft_smooth[FFT::width];
	float fft_smooth_less[FFT::width];
	float fft_integrated[FFT::width];
} g;

namespace FFT {
void onLog(ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message) {
	(void)logLevel;
	MSG("[FFT] [mal:%p:%p]\n %s", (void*)pContext, (void*)pDevice, message);
}

void onReceiveFrames(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	(void)pDevice;
	(void)pOutput;

	const float * samples = (const float *)pInput;
	for (int i = 0; i < (int)frameCount; ++i) {
		g.audio_ring_buffer[g.audio_ring_buffer_cursor] = (samples[i*2] + samples[i*2+1]) * .5f;
		g.audio_ring_buffer_cursor = (g.audio_ring_buffer_cursor + 1) % ring_buffer_len;
	}
}

bool open() {
	g.fftcfg = kiss_fftr_alloc(width * 2, false, NULL, NULL);

	ma_context_config context_config = ma_context_config_init();
	context_config.logCallback = onLog;
	ma_result result = ma_context_init(NULL, 0, &context_config, &g.context);
	if (result != MA_SUCCESS)
	{
		printf("[FFT] Failed to initialize g.context: %d", result);
		return false;
	}

	printf("[FFT] MAL g.context initialized, backend is '%s'\n", ma_get_backend_name(g.context.backend));

	ma_device_config config = ma_device_config_init(ma_device_type_capture);
	config.capture.pDeviceID = NULL;
	config.capture.format = ma_format_f32;
	config.capture.channels = 2;
	config.sampleRate = 44100;
	config.dataCallback = onReceiveFrames;
	config.pUserData = NULL;

	result = ma_device_init(&g.context, &config, &g.captureDevice);
	if (result != MA_SUCCESS)
	{
		ma_context_uninit(&g.context);
		printf("[FFT] Failed to initialize capture device: %d\n", result);
		return false;
	}

	result = ma_device_start(&g.captureDevice);
	if (result != MA_SUCCESS)
	{
		ma_device_uninit(&g.captureDevice);
		ma_context_uninit(&g.context);
		printf("[FFT] Failed to start capture device: %d\n", result);
		return false;
	}

	return true;
}

Frame read() {
	kiss_fft_cpx out[width + 1];

	const int cur = g.audio_ring_buffer_cursor;
	if (cur < width*2) {
		const int head_len = width*2 - cur;
		memcpy(g.fft_buffer_in, g.audio_ring_buffer + ring_buffer_len - head_len, head_len * sizeof(float));
		memcpy(g.fft_buffer_in+head_len, g.audio_ring_buffer, cur * sizeof(float));
	} else {
		memcpy(g.fft_buffer_in, g.audio_ring_buffer + (cur - width*2), width*2 * sizeof(float));
	}

	kiss_fftr(g.fftcfg, g.fft_buffer_in, out);

	constexpr float scaling = 1.0f / (float)width;
  const static float max_integral_value = 1024.0f;
  const float smooth_factor = 0.9f; // higher value, smoother FFT
  const float less_smooth_factor = 0.6f; // higher value, smoother FFT
	for (int i = 0; i < width; i++) {
		const float s = 2.f * sqrtf(out[i].r * out[i].r + out[i].i * out[i].i) * scaling;
		g.fft[i] = s;

		g.fft_smooth[i] = g.fft_smooth[i] * smooth_factor + (1 - smooth_factor) * s;

		g.fft_smooth_less[i] = g.fft_smooth_less[i] * less_smooth_factor + (1 - less_smooth_factor) * s;
		g.fft_integrated[i] = g.fft_integrated[i] + g.fft_smooth_less[i];
		if (g.fft_integrated[i] > max_integral_value)
			g.fft_integrated[i] -= max_integral_value;
	}

	return Frame{width, g.fft, g.fft_smooth, g.fft_integrated};
}

void close() {
	ma_device_stop(&g.captureDevice);

	ma_device_uninit(&g.captureDevice);
	ma_context_uninit(&g.context);

	kiss_fft_free(g.fftcfg);
}
} // namespace FFT
