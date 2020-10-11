#pragma once

#include "OpenGL.h"

class Texture {
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;

	friend class Framebuffer;
public:
	Texture() {
		glGenTextures(1, &name_);
	}

	Texture(const Texture &) = delete;

	Texture(Texture &&t)
		: name_(t.name_)
		, w_(t.w_)
		, h_(t.h_)
	{
		t.name_ = 0;
	}

	~Texture() {
		if (name_)
			glDeleteTextures(1, &name_);
	}

	int w() const { return w_; }
	int h() const { return h_; }

	GLuint name() const { return name_; }

	void alloc(int w, int h, PixelType pixel_type) {
		int comp = 0, type = 0, format = 0;
		switch (pixel_type) {
			case RGBA8:
				format = comp = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				break;
#ifndef ATTO_PLATFORM_RPI
			case RGBA16F:
				format = GL_RGBA;
				comp = GL_RGBA16F;
				type = GL_FLOAT;
				break;
			case RGBA32F:
				format = GL_RGBA;
				comp = GL_RGBA32F;
				type = GL_FLOAT;
				break;
#endif
		}

		upload(w, h, comp, format, type, nullptr);
	}

	void upload(int w, int h, int comp, int format, int type, const void *data) {
		w_ = w;
		h_ = h;
		GL(glBindTexture(GL_TEXTURE_2D, name_));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, format, static_cast<GLenum>(type), data));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
	}

	int bind(int slot) const {
		glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot));
		glBindTexture(GL_TEXTURE_2D, name_);
		return slot;
	}
};

