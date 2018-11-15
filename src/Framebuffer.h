#pragma once

#include "OpenGL.h"

class Framebuffer {
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;
	int targets_ = 0;

	const static GLuint draw_buffers_[4];

public:
	int w() const { return w_; }
	int h() const { return h_; }

	void init(int w, int h) { w_ = w; h_ = h; targets_ = 1; }

	Framebuffer& init() {
		glGenFramebuffers(1, &name_);
		GL(glBindFramebuffer(GL_FRAMEBUFFER, name_));
		return *this;
	}

	Framebuffer &attach(const Texture &t) {
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targets_, GL_TEXTURE_2D, t.name_, 0));
		w_ = t.w_;
		h_ = t.h_;
		++targets_;
		return *this;
	}

	void check() {
		const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
#ifdef _WIN32
			MessageBoxA(NULL, "Framebuffer is not ready", "", MB_OK);
			ExitProcess(0);
#else
			MSG("Framebuffer not ready");
#endif
		}
	}

	void bind() const {
		GL(glBindFramebuffer(GL_FRAMEBUFFER, name_));
		glViewport(0, 0, w_, h_);
		if (name_)
			glDrawBuffers(targets_, draw_buffers_);
	}
};
