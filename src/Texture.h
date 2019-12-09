#pragma once

#include "OpenGL.h"

class Texture {
	GLuint type_ = GL_TEXTURE_2D;
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;

	friend class Framebuffer;
public:
	Texture() {
		glGenTextures(1, &name_);
	}

	Texture(const Texture &) = delete;

	Texture(Texture &&t)
		: type_(t.type_)
		, name_(t.name_)
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
		upload(w, h, pixel_type, nullptr);
	}

	void upload(int w, int h, PixelType pixel_type, const void *data) {
		int comp = 0, type = 0, format = GL_RGBA;
		switch (pixel_type) {
			case RGBA8:
				comp = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				break;
#ifndef ATTO_PLATFORM_RPI
			case RGBA16F:
				comp = GL_RGBA16F;
				type = GL_FLOAT;
				break;
			case RGBA32F:
				comp = GL_RGBA32F;
				type = GL_FLOAT;
				break;
			case R32F:
				comp = GL_R32F;
				type = GL_FLOAT;
				format = GL_RED;
				break;
#endif
		}

		upload(w, h, comp, format, type, data);
	}

	int bind(int slot) {
		glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(slot));
		glBindTexture(type_, name_);
		return slot;
	}

private:
	void upload(int w, int h, int comp, int format, int type, const void *data) {
		w_ = w;
		h_ = h;
		type_ = (h < 2) ? GL_TEXTURE_1D : GL_TEXTURE_2D;
		GL(glBindTexture(type_, name_));
		if (type_ == GL_TEXTURE_2D)
			GL(glTexImage2D(type_, 0, comp, w, h, 0, format, static_cast<GLenum>(type), data));
		else
			GL(glTexImage1D(type_, 0, comp, w, 0, format, static_cast<GLenum>(type), data));
		GL(glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
	}
};

