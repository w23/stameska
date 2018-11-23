#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#define NOMSG
#include <windows.h>
#include <GL/gl.h>
#include "glext.h"
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif


#define GL_FUNC_LIST(X) \
  X(PFNGLCREATESHADERPROGRAMVPROC, CreateShaderProgramv) \
  X(PFNGLUSEPROGRAMPROC, UseProgram) \
  X(PFNGLDELETEPROGRAMPROC, DeleteProgram) \
  X(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation) \
  X(PFNGLUNIFORM1IPROC, Uniform1i) \
  X(PFNGLUNIFORM1FPROC, Uniform1f) \
  X(PFNGLUNIFORM2FPROC, Uniform2f) \
  X(PFNGLUNIFORM3FPROC, Uniform3f) \
  X(PFNGLUNIFORM4FPROC, Uniform4f) \
  X(PFNGLGENFRAMEBUFFERSPROC, GenFramebuffers) \
  X(PFNGLBINDFRAMEBUFFERPROC, BindFramebuffer) \
  X(PFNGLFRAMEBUFFERTEXTURE2DPROC, FramebufferTexture2D) \
  X(PFNGLDRAWBUFFERSPROC, DrawBuffers) \
  X(PFNGLACTIVETEXTUREPROC, ActiveTexture) \
  X(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog) \
  X(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog) \
  X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, CheckFramebufferStatus) \

#ifdef _WIN32
#ifndef GL_DECLARE_FUNC 
#define GL_DECLARE_FUNC(type, name) extern type gl ## name;
#endif
GL_FUNC_LIST(GL_DECLARE_FUNC)
#undef GL_DECLARE_FUNC

#define GL_LOAD_FUNC(type, name) gl ## name = (type)wglGetProcAddress("gl" # name);
#define GL_LOAD_FUNCS GL_FUNC_LIST(GL_LOAD_FUNC)

#else // ifndef _WIN32
#define GL_LOAD_FUNCS
#endif

#ifdef TOOL
//#define DEBUG_GL
#endif

#ifndef DEBUG_GL
#define GL(f) f
#define glGetError()
#else
static const char *a__GlPrintError(int error) {
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
#define GL(f) \
	do { \
		f;\
		GLCHECK(#f); \
	} while(0)
static void GLCHECK(const char *func) {
	const int glerror = glGetError();
	if (glerror != GL_NO_ERROR) {
#ifdef _WIN32
		MessageBoxA(NULL, a__GlPrintError(glerror), func, 0);
		ExitProcess(0);
#else
		printf("%s\n", a__GlPrintError(glerror));
		abort();
#endif
	};
}
#endif /* DEBUG_GL */
