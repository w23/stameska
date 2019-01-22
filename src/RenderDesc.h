#pragma once

#include <string>
#include <vector>

namespace renderdesc {

struct Texture {
	int w, h;
	enum PixelType {
		RGBA8,
		RGBA32F,
		RGBA16F,
		//Depth24,
	} pixel_type;

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
		int start, count;

		DrawArrays() {}
		DrawArrays(int start, int count) : start(start), count(count) {}
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

struct Pipeline {
	std::vector<Texture> textures;
	std::vector<Framebuffer> framebuffers;
	std::vector<std::string> shader_filenames;
	std::vector<Program> programs;

	std::vector<Command> commands;
};

/*
	framebuffer: part
		size: [512, 512]
		pingpong: 1
		textures:
			- pos: rgba32f
			- vel: rgba32f
			- col: rgba32f
	framebuffer: frame
		size: [$width, $height]
		textures:
			- color: rgba16f
			- depth: d24
	programs:
		- particle_update:
			fragment: pufrg.glsl
			vertex: puvtx.glsl
		... 
	
	paint:
		- bindFramebuffer: part
		- useProgram: particle_update
		- bindTextures:
			pp: part.pos
			pv: part.vel
			pc: part.col
		- drawFullscreen
		- bindFramebuffer: frame
		- clear: 
			color: [0, 0, 0, 0]
			depth: 1
		- enable: depth
		- useProgram: particles_draw
		- bindTextures:
			pp: part.pos
			pv: part.vel
			pc: part.col
		- drawArrays: { start: 0, count: part.pixel_count}
		- bindFramebuffer: 0 
		- useProgram: postfx
		- setUniforms:
			frame: [sampler2D, frame.color]
		- drawFullscreen
*/

} // namespace renderdesc 
