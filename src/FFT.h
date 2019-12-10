#pragma once
#include <vector>

namespace FFT {
	constexpr int width = 1024;

	struct Frame {
		int len;
		const float *fft;
		const float *fft_smooth;
		const float *fft_integrated;
	};

	bool open();
	Frame read();
	int samples();
	void close();
}
