#pragma once

#include "PolledResource.h"
#include "PolledFile.h"
#include "Texture.h"
#include "OpenGL.h"

class PolledTexture : public PolledResource {
public:
	PolledTexture(std::shared_ptr<PolledFile> file) : file_(file) {}
	PolledTexture(int w, int h, PixelType pixel_type) {
		texture_.alloc(w, h, pixel_type);
	}

	bool poll(unsigned int poll_seq);
	const Texture &texture() const { return texture_; }

private:
	std::shared_ptr<PolledFile> file_;
	Texture texture_;
};
