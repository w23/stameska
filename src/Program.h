#pragma once

#include "OpenGL.h"
#include "utils.h"

#ifndef TOOL
#define MSG(str) MessageBoxA(NULL, str, "Error", MB_OK)
#else
#include "atto/app.h"
#define MSG(...) aAppDebugPrintf(__VA_ARGS__)
#endif

class Program {
protected:
	GLuint name = 0;

	static GLuint createFragmentProgram(const ConstArrayView<const char*>& sources) {
		return createSeparableProgram(GL_FRAGMENT_SHADER, sources);
	}

	static GLuint createSeparableProgram(GLenum type, const ConstArrayView<const char*>& sources) {
		const GLuint pid = glCreateShaderProgramv(type, sources.size(), sources.ptr());

#define SHADER_DEBUG
#ifdef SHADER_DEBUG
		{
			int result;
			char info[2048];
#ifdef _WIN32
#define glGetObjectParameterivARB ((PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB"))
#define glGetInfoLogARB ((PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB"))
#endif
			glGetObjectParameterivARB(pid, GL_OBJECT_LINK_STATUS_ARB, &result);
			glGetInfoLogARB(pid, 2047, NULL, (char*)info);
			if (!result)
			{
				MSG(info);
				//MessageBoxA(NULL, info, "LINK", 0x00000000L);
#ifndef TOOL
				ExitProcess(0);
#endif
			}
		}
#endif

		return pid;
	}

public:
	void load(const ConstArrayView<const char*>& sources) {
		name = createFragmentProgram(sources);
	}

	bool use(int w, int h, float t) const {
		if (!name)
			return false;

		glUseProgram(name);
		//glUniform1f(glGetUniformLocation(name, "t"), t);
		glUniform1i(glGetUniformLocation(name, "s"), (int)t);
		glUniform2f(glGetUniformLocation(name, "R"), w, h);
		return true;
	}

	const Program& setUniform(const char *uname, int i) const {
		glUniform1i(glGetUniformLocation(name, uname), i);
		return *this;
	}

	const Program& setUniform(const char *uname, float f) const {
		glUniform1f(glGetUniformLocation(name, uname), f);
		return *this;
	}

	const Program& setUniform(const char *uname, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(name, uname), x, y, z, w);
		return *this;
	}

	void compute() const {
		glRects(-1, -1, 1, 1);
	}
};
