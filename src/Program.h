#pragma once

#include "ShaderSource.h"
#include "Expected.h"
#include "utils.h"
#include "OpenGL.h"

#include <string>
#include <utility>

class Program {
public:
	Program() {}
	~Program() {}
	Program(Program&&) = default;
	Program(const Program &) = delete;

	bool valid() const { return !!handle_.name(); }

	static Expected<Program,std::string> create(const std::string& fragment, const std::string &vertex);

	const Program& use() const;

	const Program& setUniform(const char *uname, int i) const {
		glUniform1i(glGetUniformLocation(handle_.name(), uname), i);
		return *this;
	}

	const Program& setUniform(const char *uname, float x) const {
		glUniform1f(glGetUniformLocation(handle_.name(), uname), x);
		return *this;
	}

	const Program& setUniform(const char *uname, float x, float y) const {
		glUniform2f(glGetUniformLocation(handle_.name(), uname), x, y);
		return *this;
	}

	const Program& setUniform(const char *uname, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(handle_.name(), uname), x, y, z);
		return *this;
	}

	const Program& setUniform(const char *uname, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(handle_.name(), uname), x, y, z, w);
		return *this;
	}

	Program& operator=(Program&&) = default;

private:
	class Handle {
	public:
		Handle() : name_(0) {}
		Handle(GLuint name) : name_(name) {}
		Handle(const Handle &) = delete;
		Handle(Handle &&);
		Handle& operator=(Handle&&);
		~Handle();

		GLuint name() const { return name_; }

	private:
		GLuint name_;
	};

	Program(Handle &&handle) : handle_(std::move(handle)) {}

	Handle handle_;
};
