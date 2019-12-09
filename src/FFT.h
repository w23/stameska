#pragma once
#include <vector>

namespace FFT {
	constexpr int width = 1024;

	struct Frame {
		std::vector<float> fft;
		/*
		std::vector<float> ffts; // smoothed
		std::vector<float> ffti; // integrated
		*/
	};

	bool open();
	bool read(Frame &);
	void close();
}
