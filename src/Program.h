#pragma once

#include "ShaderSource.h"
#include "utils.h"
#include "OpenGL.h"

#include <utility>

class Program {
public:
	Program() {}
	~Program() {}
	Program(Program&&) = default;

	bool valid() const { return handle_.valid(); }

	struct Sources {
		const shader::Sources *vertex = nullptr;
		const shader::Sources *fragment = nullptr;
	};

	static Program load(Sources sources) {
		return Program(create(sources));
	}

	static Program load(const shader::Sources& sources) {
		return Program(createFragmentProgram(sources));
	}

	const Program& use() const {
		handle_.use();
		return *this;
	}

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

	void compute() const {
		glRects(-1, -1, 1, 1);
	}

	Program& operator=(Program&&) = default;

private:
	Program(const Program&) = delete;

	class Handle {
		Handle(const Handle&) = delete;
		GLuint name_;

	public:
		Handle() : name_(0) {}
		explicit Handle(GLuint name) : name_(name) {}
		Handle(Handle&& other) : name_(other.name_) { other.name_ = 0; }
		~Handle() {
			if (name_ > 0)
				glDeleteProgram(name_);
		}

		Handle& operator=(Handle&& other) {
			if (name_ > 0)
				glDeleteProgram(name_);

			name_ = other.name_;
			other.name_ = 0;
			return *this;
		}

		bool valid() const { return name_ > 0; }

		GLuint name() const {
			return name_;
		}

		bool use() const {
			if (name_ < 1)
				return false;

			glUseProgram(name_);
			return true;
		}
	};

	Handle handle_;

	class Shader {
	public:
		Shader(const Shader&) = delete;

		bool isValid() const { return name_ != 0; }

		GLuint name() const { return name_; }

		Shader(GLuint kind, const shader::Sources& src)
			: name_(0)
		{
			name_ = glCreateShader(kind);
			GLchar const * const c_src = src.source().c_str();
			glShaderSource(name_, 1, &c_src, nullptr);
			glCompileShader(name_);

			GLint compiled = GL_FALSE;
			glGetShaderiv(name_, GL_COMPILE_STATUS, &compiled);
			if (compiled != GL_TRUE) {
				GLsizei length = 0;
				char info[2048];
				glGetShaderInfoLog(name_, COUNTOF(info), &length, info);
				MSG("Shader compilation error: %s", info);
				glDeleteShader(name_);
				name_ = 0;
			}
		}

		~Shader() {
			if (name_ != 0)
				glDeleteShader(name_);
		}

	private:
		GLuint name_;
	};

	static Handle create(Sources sources) {
		Handle pid = Handle(glCreateProgram());
		if (!sources.vertex || !sources.fragment)
			return Handle();
		Shader vertex(GL_VERTEX_SHADER, *sources.vertex);
		if (!vertex.isValid())
			return Handle();
		Shader fragment(GL_FRAGMENT_SHADER, *sources.fragment);
		if (!fragment.isValid())
			return Handle();

		glAttachShader(pid.name(), vertex.name());
		glAttachShader(pid.name(), fragment.name());

		glLinkProgram(pid.name());

		GLint linked = GL_FALSE;
		glGetProgramiv(pid.name(), GL_LINK_STATUS, &linked);
		if (linked != GL_TRUE) {
			GLsizei length = 0;
			char info[2048];
			glGetProgramInfoLog(pid.name(), COUNTOF(info), &length, info);
			MSG("Program link error: %s", info);
			return Handle();
		}

		return pid;
	}

	static Handle createFragmentProgram(const shader::Sources& sources) {
		return createSeparableProgram(GL_FRAGMENT_SHADER, sources.source().c_str());
	}

	static Handle createSeparableProgram(GLenum type, const char* sources) {
		Handle pid = Handle(glCreateShaderProgramv(type, 1, &sources));

		if (!pid.valid())
			return Handle();

		{
			int result;
			char info[2048];
#ifdef _WIN32
#define glGetObjectParameterivARB ((PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB"))
#define glGetInfoLogARB ((PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB"))
#endif
			glGetObjectParameterivARB(pid.name(), GL_OBJECT_LINK_STATUS_ARB, &result);
			glGetInfoLogARB(pid.name(), 2047, NULL, (char*)info);
			if (!result)
			{
				MSG("Shader error: %s", info);
				return Handle();
			}
		}

		return pid;
	}

	Program(Handle&& handle) : handle_(std::move(handle)) {}
};
