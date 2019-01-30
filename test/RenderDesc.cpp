#include "RenderDesc.h"

#include <stdio.h>

template <typename T>
std::string toString(T t);

std::string toString(const std::string& t) { return t; }

template <>
std::string toString(const char *t) { return t; }

template <>
std::string toString(int i) {
	char buffer[32] = {'\0'};
	snprintf(buffer, sizeof(buffer), "%d", i);
	return buffer;
}

template <>
std::string toString(unsigned long i) {
	char buffer[32] = {'\0'};
	snprintf(buffer, sizeof(buffer), "%lu", i);
	return buffer;
}

using namespace renderdesc;

template <>
std::string toString(Command::Op op) {
	switch (op) {
#define X(t, n) case Command::Op::t: return #t;
		LIST_COMMANDS(X)
#undef X
	}

	return "INVALID OP";
}

template <>
std::string toString(Command::Index::Pingpong pp) {
	switch(pp) {
		case Command::Index::Pingpong::Fixed: return "Fixed";
		case Command::Index::Pingpong::Ping: return "Ping";
		case Command::Index::Pingpong::Pong: return "Pong";
	}

	return "INVALID PP";
}

#define CHECK_EQUAL(a, b) \
	if ((a) != (b)) { \
		fprintf(stderr, "%s != %s'\n", toString(a).c_str(), toString(b).c_str()); \
		return 1; \
	}

static int simple() {
	const std::string source =
	"programs:\n"
	"  main:\n"
	"    fragment: fragment.glsl\n"
	"    vertex: vertex.glsl\n"
	"paint:\n"
	" - useProgram: main\n"
	" - drawFullscreen\n";

	const Pipeline p = Pipeline(source);

	CHECK_EQUAL(p.textures.size(), 0);
	CHECK_EQUAL(p.framebuffers.size(), 0);
	CHECK_EQUAL((int)p.shader_filenames.size(), 2);
	CHECK_EQUAL((int)p.programs.size(), 1);
	CHECK_EQUAL((int)p.commands.size(), 2);

	CHECK_EQUAL(p.shader_filenames[p.programs[0].vertex], "vertex.glsl");
	CHECK_EQUAL(p.shader_filenames[p.programs[0].fragment], "fragment.glsl");

	CHECK_EQUAL(p.commands[0].op, Command::Op::UseProgram);
	CHECK_EQUAL(p.commands[0].useProgram.program.index, 0);
	CHECK_EQUAL(p.commands[0].useProgram.program.pingpong, Command::Index::Pingpong::Fixed);

	CHECK_EQUAL(p.commands[1].op, Command::Op::DrawFullscreen);

	return 0;
}

int testRenderDesc() {
	return simple();
}
