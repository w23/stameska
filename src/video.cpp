#include "video.h"

#include "Program.h"

//#include "pcg32.h"
//#include "Texture.h"
//#include "Framebuffer.h"

#include "PolledFile.h"

#include <memory>

/* FIXME
const GLuint Framebuffer::draw_buffers_[4] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};

#ifdef DO_NOISE
#define NOISE_SIZE 1024
uint32_t noise_buffer[NOISE_SIZE * NOISE_SIZE];
static void initNoise(Texture &noise) {
	pcg32_random_t rand = PCG32_INITIALIZER;
	for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i) {
		noise_buffer[i] = pcg32_random_r(&rand);
	}

	noise.upload(NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise_buffer);
}
#endif
*/

static struct {
	struct {
		int w, h;
	} output;

	std::unique_ptr<PolledFile> shader_file;
	std::unique_ptr<PollAdaptor> program_adaptor;
	std::unique_ptr<PolledProgram<1>> program;
} state;

#ifdef TOOL
void video_tool_resize(int w, int h) {
	state.output.w = w;
	state.output.h = h;
	glViewport(0, 0, w, h);
}
#endif

void video_init(int w, int h, const char *shader) {
	state.output.w = w;
	state.output.h = h;

	GL_LOAD_FUNCS

	state.shader_file.reset(new PolledFile(shader));
	state.program_adaptor.reset(new PollAdaptor(*state.shader_file.get()));
	state.program.reset(new PolledProgram<1>(state.program_adaptor.get()));

	state.shader_file->poll();
	state.program->poll();
}

void video_paint(float tick) {
	state.shader_file->poll();
	state.program->poll();
	state.program->use(state.output.w, state.output.h, tick);
	glRects(-1, -1, 1, 1);
}
