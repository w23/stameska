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
	X(CREATESHADERPROGRAMV, CreateShaderProgramv) \
	X(CREATEPROGRAM, CreateProgram) \
	X(CREATESHADER, CreateShader) \
	X(SHADERSOURCE, ShaderSource) \
	X(COMPILESHADER, CompileShader) \
	X(GETSHADERIV, GetShaderiv) \
	X(DELETESHADER, DeleteShader) \
	X(ATTACHSHADER, AttachShader) \
	X(LINKPROGRAM, LinkProgram) \
	X(GETPROGRAMIV, GetProgramiv) \
	X(USEPROGRAM, UseProgram) \
	X(DELETEPROGRAM, DeleteProgram) \
	X(GETUNIFORMLOCATION, GetUniformLocation) \
	X(UNIFORM1I, Uniform1i) \
	X(UNIFORM1F, Uniform1f) \
	X(UNIFORM2F, Uniform2f) \
	X(UNIFORM3F, Uniform3f) \
	X(UNIFORM4F, Uniform4f) \
	X(GENFRAMEBUFFERS, GenFramebuffers) \
	X(BINDFRAMEBUFFER, BindFramebuffer) \
	X(FRAMEBUFFERTEXTURE2D, FramebufferTexture2D) \
	X(DRAWBUFFERS, DrawBuffers) \
	X(ACTIVETEXTURE, ActiveTexture) \
	X(GETPROGRAMINFOLOG, GetProgramInfoLog) \
	X(GETSHADERINFOLOG, GetShaderInfoLog) \
	X(CHECKFRAMEBUFFERSTATUS, CheckFramebufferStatus) \
	X(DISABLEVERTEXATTRIBARRAY, DisableVertexAttribArray) \
	X(ENABLEVERTEXATTRIBARRAY, EnableVertexAttribArray) \
	X(GETATTRIBLOCATION, GetAttribLocation) \
	X(VERTEXATTRIBPOINTER, VertexAttribPointer) \

#ifdef _WIN32
#ifndef GL_DECLARE_FUNC
#define GL_DECLARE_FUNC(type, name) extern PFNGL ## type ## PROC gl ## name;
#endif
GL_FUNC_LIST(GL_DECLARE_FUNC)
#undef GL_DECLARE_FUNC

#define GL_LOAD_FUNC(type, name) gl ## name = (PFNGL ## type ## PROC)wglGetProcAddress("gl" # name);
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
#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#endif
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
		printf("%s: %s\n", func, a__GlPrintError(glerror));
		abort();
#endif
	};
}
#endif /* DEBUG_GL */
