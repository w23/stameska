// This file is mostly taken as-is from Bonzomatic by Gargaj et al.
#include "utils.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <kiss_fft.h>
#include <tools/kiss_fftr.h>

#include <stdio.h>
#include <memory.h>

#include "FFT.h"

constexpr int ringBufferSize = FFT::width * 4;

static struct {
	kiss_fftr_cfg fftcfg;
	ma_context context;
	ma_device captureDevice;
	int audioRingBufferCursor = 0;
	float audioRingBuffer[ringBufferSize];
	/*
	float fftBufferIn[FFT::width];
	float fft[FFT::width];
	float ffts[FFT::width];
	float ffts_i[FFT::width];
	float ffti[FFT::width];
	*/
} g;

namespace FFT {

void OnLog(ma_context* pContext, ma_device* pDevice, ma_uint32 logLevel, const char* message)
{
	(void)logLevel;
	MSG("[FFT] [mal:%p:%p]\n %s", (void*)pContext, (void*)pDevice, message);
}

void OnReceiveFrames(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	(void)pDevice;
	(void)pOutput;

	const float * samples = (const float *)pInput;
	for (int i = 0; i < (int)frameCount; ++i) {
		g.audioRingBuffer[g.audioRingBufferCursor] = (samples[i*2] + samples[i*2+1]) * .5f;
		g.audioRingBufferCursor = (g.audioRingBufferCursor + 1) % ringBufferSize;
	}
}

bool open() {
	g.fftcfg = kiss_fftr_alloc(width * 2, false, NULL, NULL);

	ma_context_config context_config = ma_context_config_init();
	context_config.logCallback = OnLog;
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
	config.dataCallback = OnReceiveFrames;
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

bool read(Frame &frame)
{
	float in[width*2];
	kiss_fft_cpx out[width + 1];

	const int cur = g.audioRingBufferCursor;
	if (cur < width*2) {
		const int head_len = width*2 - cur;
		memcpy(in, g.audioRingBuffer + ringBufferSize - head_len, head_len * sizeof(float));
		memcpy(in+head_len, g.audioRingBuffer, cur * sizeof(float));
	} else {
		memcpy(in, g.audioRingBuffer + (cur - width*2), width*2 * sizeof(float));
	}

	kiss_fftr(g.fftcfg, in, out);

	if (frame.fft.size() != width)
		frame.fft.resize(width, 0.f);
	/*
	if (frame.ffts.size() != width)
		frame.ffts.resize(width);
	if (frame.ffti.size() != width)
		frame.ffti.resize(width);
	*/

	constexpr float scaling = 1.0f / (float)width;
	/*
  const static float maxIntegralValue = 1024.0f;
  const float fFFTSmoothingFactor = 0.9f; // higher value, smoother FFT
  const float fFFTSlightSmoothingFactor = 0.6f; // higher value, smoother FFT
	*/
	for (int i = 0; i < width; i++) {
		const float s = 2.f * sqrtf(out[i].r * out[i].r + out[i].i * out[i].i) * scaling;
		frame.fft[i] = s;

		/*
		fftDataSmoothed[i] = fftDataSmoothed[i] * fFFTSmoothingFactor + (1 - fFFTSmoothingFactor) * fftData[i];

		fftDataSlightlySmoothed[i] = fftDataSlightlySmoothed[i] * fFFTSlightSmoothingFactor + (1 - fFFTSlightSmoothingFactor) * fftData[i];
		fftDataIntegrated[i] = fftDataIntegrated[i] + fftDataSlightlySmoothed[i];
		if (fftDataIntegrated[i] > maxIntegralValue) {
			fftDataIntegrated[i] -= maxIntegralValue;
		}
		*/
	}

	return true;
}

void close() {
	ma_device_stop(&g.captureDevice);

	ma_device_uninit(&g.captureDevice);
	ma_context_uninit(&g.context);

	kiss_fft_free(g.fftcfg);
}
} // namespace FFT
