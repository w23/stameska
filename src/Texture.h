#pragma once

#include "OpenGL.h"

class Texture {
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;

	friend class Framebuffer;
public:
	void init() { glGenTextures(1, &name_); }
	void upload(int w, int h, int comp, int type, const void *data) {
		w_ = w;
		h_ = h;
		GL(glBindTexture(GL_TEXTURE_2D, name_));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);// data ? GL_REPEAT : GL_CLAMP);
	}

	int bind(int slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, name_);
		return slot;
	}
};

