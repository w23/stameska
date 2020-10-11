#include "PolledTexture.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool PolledTexture::poll(unsigned int poll_seq) {
	if (!beginUpdate(poll_seq))
		return false;

	if (!file_ || !file_->poll(poll_seq)) {
		endUpdate();
		return false;
	}

	const auto &data = file_->data();

	int w, h, n;
	unsigned char *pixels = stbi_load_from_memory(data.data(), data.size(), &w, &h, &n, 0);
	if (!pixels) {
		MSG("Could not parse texture (FIXME filename)");
		endUpdate();
		return false;
	}

	switch (n) {
		case 1: n = GL_LUMINANCE; break;
		case 2: n = GL_LUMINANCE_ALPHA; break;
		case 3: n = GL_RGB; break;
		case 4: n = GL_RGBA; break;
		default:
			MSG("Invalid component count from stbi: %d", n);
			endUpdate();
			return false;
	}

	texture_.upload(w, h, n, n, GL_UNSIGNED_BYTE, pixels);
	stbi_image_free(pixels);
	return endUpdate();
}
