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
	X(ACTIVETEXTURE, ActiveTexture) \
	X(ATTACHSHADER, AttachShader) \
	X(BINDFRAMEBUFFER, BindFramebuffer) \
	X(BINDRENDERBUFFER, BindRenderbuffer) \
	X(CHECKFRAMEBUFFERSTATUS, CheckFramebufferStatus) \
	X(COMPILESHADER, CompileShader) \
	X(CREATEPROGRAM, CreateProgram) \
	X(CREATESHADER, CreateShader) \
	X(CREATESHADERPROGRAMV, CreateShaderProgramv) \
	X(DELETEFRAMEBUFFERS, DeleteFramebuffers) \
	X(DELETEPROGRAM, DeleteProgram) \
	X(DELETESHADER, DeleteShader) \
	X(DISABLEVERTEXATTRIBARRAY, DisableVertexAttribArray) \
	X(DRAWBUFFERS, DrawBuffers) \
	X(ENABLEVERTEXATTRIBARRAY, EnableVertexAttribArray) \
	X(FRAMEBUFFERRENDERBUFFER, FramebufferRenderbuffer) \
	X(FRAMEBUFFERTEXTURE2D, FramebufferTexture2D) \
	X(GENFRAMEBUFFERS, GenFramebuffers) \
	X(GENRENDERBUFFERS, GenRenderbuffers) \
	X(GETATTRIBLOCATION, GetAttribLocation) \
	X(GETINFOLOGARB, GetInfoLogARB) \
	X(GETOBJECTPARAMETERIVARB, GetObjectParameterivARB) \
	X(GETPROGRAMINFOLOG, GetProgramInfoLog) \
	X(GETPROGRAMIV, GetProgramiv) \
	X(GETSHADERINFOLOG, GetShaderInfoLog) \
	X(GETSHADERIV, GetShaderiv) \
	X(GETUNIFORMLOCATION, GetUniformLocation) \
	X(LINKPROGRAM, LinkProgram) \
	X(RENDERBUFFERSTORAGE, RenderbufferStorage) \
	X(SHADERSOURCE, ShaderSource) \
	X(UNIFORM1F, Uniform1f) \
	X(UNIFORM1I, Uniform1i) \
	X(UNIFORM2F, Uniform2f) \
	X(UNIFORM3F, Uniform3f) \
	X(UNIFORM4F, Uniform4f) \
	X(USEPROGRAM, UseProgram) \
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

#define DEBUG_GL

#ifndef DEBUG_GL
#define GL(f) f
#define glGetError()
#else
#define GL(f) \
	do { \
		f;\
		GLCHECK(#f); \
	} while(0)

const char *a__GlPrintError(int error);
void GLCHECK(const char *func);
#endif /* DEBUG_GL */

enum PixelType {
	RGBA8,
	RGBA16F,
	RGBA32F,
	//Depth24,
};

