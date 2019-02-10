#pragma once
#include "Expected.h"
#include "OpenGL.h"

#include <string>
#include <vector>

namespace renderdesc {

struct Texture {
	int w, h;
	PixelType pixel_type;

	Texture(int w, int h, PixelType type) : w(w), h(h), pixel_type(type) {}
};

#define MAX_TARGET_TEXTURES 4

struct Framebuffer {
	int textures_count;

	// indices into Pipeline::textures
	int textures[MAX_TARGET_TEXTURES];
};

struct Program {
	// indices into Pipeline::shader_filenames
	int vertex, fragment;

	Program(int vertex, int fragment) : vertex(vertex), fragment(fragment) {}
};

class Command {
public:

	// Index into corresponding array in Pipeline
	struct Index {
		int index;
		enum Pingpong {
			Fixed, // Real index is always index
			Ping, // real index is index + (frame number is even)?0:1
			Pong  // real index is index + (frame number is even)?1:0
		} pingpong;

		Index() : index(-1) {}
		Index(int index, Pingpong pingpong = Fixed) : index(index), pingpong(pingpong) {}
	};

	enum class Flag {
		DepthTest,
		VertexProgramPointSize,
	};

	struct BindFramebuffer {
		// >= 0 index, < 0 == no framebuffer
		Index framebuffer;

		BindFramebuffer() {}
		BindFramebuffer(Index framebuffer) : framebuffer(framebuffer) {}
	};

	struct UseProgram {
		Index program;

		UseProgram() {}
		UseProgram(Index program) : program(program) {}
	};

	struct BindTexture {
		std::string name;
		Index texture;

		BindTexture() {}
		BindTexture(std::string_view name, Index texture)
			: name(name)
			, texture(texture)
		{
		}
	};

	struct Clear {
		float r, g, b, a;
		bool depth;

		Clear() {}
		Clear(float r, float g, float b, float a, bool depth)
			: r(r), g(g), b(b), a(a), depth(depth) {}
	};

	struct Enable {
		Flag flag;

		Enable() {}
		Enable(Flag flag) : flag(flag) {}
	};

	struct Disable {
		Flag flag;

		Disable() {}
		Disable(Flag flag) : flag(flag) {}
	};

	struct DrawArrays {
		enum class Mode {
			Points = GL_POINTS,
			Triangles = GL_TRIANGLES,
			TriangleStrip = GL_TRIANGLE_STRIP,
			TriangleFan = GL_TRIANGLE_FAN,
			Lines = GL_LINES,
			LineStrip = GL_LINE_STRIP,
			LineLoop = GL_LINE_LOOP,
		};

		Mode mode;
		int start, count;

		DrawArrays() {}
		DrawArrays(Mode mode, int start, int count) : mode(mode), start(start), count(count) {}
	};

	struct DrawFullscreen {};

#define LIST_COMMANDS(X) \
		X(BindFramebuffer, bindFramebuffer) \
		X(UseProgram, useProgram) \
		X(BindTexture, bindTexture) \
		X(DrawFullscreen, drawFullscreen) \
		X(Clear, clear) \
		X(Enable, enable) \
		X(Disable, disable) \
		X(DrawArrays, drawArrays) \

	enum class Op {
#define X(t, n) t,
		LIST_COMMANDS(X)
#undef X
	} op;

#define X(t, n) t n;
	LIST_COMMANDS(X)
#undef X

#define X(t, n) \
	Command(t n) : op(Op::t), n(n) {}
	LIST_COMMANDS(X)
#undef X
};

class Pipeline {
public:
	std::vector<Texture> textures;
	std::vector<Framebuffer> framebuffers;
	std::vector<std::string> shader_filenames;
	std::vector<Program> programs;

	std::vector<Command> commands;

	static Expected<Pipeline, std::string> load(std::string_view s);
};

} // namespace renderdesc
