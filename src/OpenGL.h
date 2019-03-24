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
	X(DELETEFRAMEBUFFERS, DeleteFramebuffers) \
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
	X(GETOBJECTPARAMETERIVARB, GetObjectParameterivARB) \
	X(GETINFOLOGARB, GetInfoLogARB) \
	X(GENRENDERBUFFERS, GenRenderbuffers) \
	X(BINDRENDERBUFFER, BindRenderbuffer) \
	X(RENDERBUFFERSTORAGE, RenderbufferStorage) \
	X(BINDRENDERBUFFER, BindRenderbuffer) \
	X(FRAMEBUFFERRENDERBUFFER, FramebufferRenderbuffer) \

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

