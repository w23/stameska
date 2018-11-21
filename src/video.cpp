#include "video.h"

#include "Program.h"

//#include "pcg32.h"
//#include "Texture.h"
//#include "Framebuffer.h"

#include "ShaderSource.h"
#include "PolledFile.h"

#include <memory>
#include <vector>

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

class PolledShaderSource : public PolledResource {
public:
	PolledShaderSource(const std::shared_ptr<PolledFile>& file)
		: file_(file)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq) && file_.poll(poll_seq)) {
			try {
				std::vector<shader::Source> sources;
				sources.emplace_back(shader::Source::load(file_->string()));
				sources_ = shader::Sources::load(sources);
				return endUpdate();
			}	catch (const std::runtime_error& e) {
				MSG("Error updating shader source: %s", e.what());
			}
		}

		return false;
	}

	const shader::Sources& get() const { return sources_; }

private:
	PollMux<PolledFile> file_;
	shader::Sources sources_;
};

class PolledShaderProgram : public PolledResource {
public:
	PolledShaderProgram(const std::shared_ptr<PolledShaderSource>& source)
		: source_(source)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq) && source_->poll(poll_seq)) {
			try {
				program_ = Program::load(source_->get());
				return endUpdate();
			} catch (const std::runtime_error& e) {
				MSG("Error updating program: %s", e.what());
			}
		}

		return false;
	}

	const Program& get() const { return program_; }
	const Program& operator->() const { return program_; }

private:
	const std::shared_ptr<PolledShaderSource> source_;
	Program program_;
};

static struct {
	struct {
		int w, h;
	} output;

	std::shared_ptr<PolledFile> shader_file;
	std::shared_ptr<PolledShaderSource> shader_source;
	std::unique_ptr<PolledShaderProgram> program;
} state;

void video_tool_resize(int w, int h) {
	state.output.w = w;
	state.output.h = h;
	glViewport(0, 0, w, h);
}

void video_init(int w, int h, const char *shader) {
	state.output.w = w;
	state.output.h = h;

	GL_LOAD_FUNCS

	state.shader_file.reset(new PolledFile(shader));
	state.shader_source.reset(new PolledShaderSource(state.shader_file));
	state.program.reset(new PolledShaderProgram(state.shader_source));
}

void video_paint(float tick) {
	static unsigned int poll_seq = 0;
	poll_seq++;

	state.program->poll(poll_seq);

	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (state.program->get().valid())
		state.program->get().use()
			.setUniform("R", state.output.w, state.output.h)
			.setUniform("t", tick)
			.compute();
}
