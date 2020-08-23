#define GL_DECLARE_FUNC(type, name) PFNGL ## type ## PROC gl ## name;
#include "OpenGL.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#endif

const char *a__GlPrintError(int error) {
	const char *errstr = "UNKNOWN";
	switch (error) {
	case GL_INVALID_ENUM: errstr = "GL_INVALID_ENUM"; break;
	case GL_INVALID_VALUE: errstr = "GL_INVALID_VALUE"; break;
	case GL_INVALID_OPERATION: errstr = "GL_INVALID_OPERATION"; break;
#ifdef GL_STACK_OVERFLOW
	case GL_STACK_OVERFLOW: errstr = "GL_STACK_OVERFLOW"; break;
#endif
#ifdef GL_STACK_UNDERFLOW
	case GL_STACK_UNDERFLOW: errstr = "GL_STACK_UNDERFLOW"; break;
#endif
	case GL_OUT_OF_MEMORY: errstr = "GL_OUT_OF_MEMORY"; break;
#ifdef GL_TABLE_TOO_LARGE
	case GL_TABLE_TOO_LARGE: errstr = "GL_TABLE_TOO_LARGE"; break;
#endif
	case 1286: errstr = "INVALID FRAMEBUFFER"; break;
	};
	return errstr;
}

void GLCHECK(const char *func) {
	const int glerror = glGetError();
	if (glerror != GL_NO_ERROR) {
#ifdef _WIN32
		MessageBoxA(NULL, a__GlPrintError(glerror), func, 0);
		ExitProcess(0);
#else
		printf("%s: %s\n", func, a__GlPrintError(glerror));
		abort();
#endif
	}
}
