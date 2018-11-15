#include "video.h"

#include "Program.h"
#include "pcg32.h"
#include "Texture.h"
#include "Framebuffer.h"

#include "PolledFile.h"

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

#ifdef TOOL
#define DECLARE_POLLED_SHADER_FILE(s) PolledFile s("shaders/" # s ".glsl");
SHADERS(DECLARE_POLLED_SHADER_FILE)
#undef DECLARE_POLLED_SHADER_FILE

#define DECLARE_PROGRAM(p, ...) \
	PollAdaptor adaptors_##p[] = {__VA_ARGS__}; \
	PolledProgram<COUNTOF(adaptors_##p)> prog_##p(adaptors_##p);
PROGRAMS(DECLARE_PROGRAM)
#undef DECLARE_PROGRAM

#else
#define DECLARE_PROGRAM(p, ...) \
	const char *p ## _sources[] = {__VA_ARGS__}; \
	Program prog_##p;
PROGRAMS(DECLARE_PROGRAM)
#undef DECLARE_PROGRAM
#endif

//Texture text;
//Texture noise;
Texture frame;

Framebuffer screen;
Framebuffer framebuffer;

int w, h;
#ifdef TOOL
void video_tool_resize(int W, int H) {
	w = W; h = H;
	screen.init(w, h);
}
#endif
void video_init(int W, int H) {
	w = W; h = H;

	GL_LOAD_FUNCS

//	text.init();
//	noise.init();
	frame.init();
	frame.upload(640, 720, GL_RGBA16F, GL_FLOAT, nullptr);
	screen.init(w, h);
	framebuffer.init().attach(frame).check();

//	initNoise(noise);

#ifdef TOOL
#define POLL_SHADER_FILE(s) s.poll();
SHADERS(POLL_SHADER_FILE)
#undef INIT_PROGRAM

#else
#define LOAD_PROGRAM(p, ...) prog_##p.load(ConstArrayView<const char*>(p##_sources, COUNTOF(p##_sources)));
PROGRAMS(LOAD_PROGRAM)
#undef LOAD_PROGRAM
#endif

//	initText(text);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//static float gt;

static const Program& renderPass(float tick, const Framebuffer& fb, const Program& p) {
	fb.bind();
	glViewport(0, 0, fb.w(), fb.h());

	int tslot = 0;
	p.use(fb.w(), fb.h(), tick);
//	p.setUniform("N", noise.bind(tslot++));
	p.setUniform("F", frame.bind(tslot++));
//	p.setUniform("T", text.bind(tslot++));

	/*
	for (int i = 0; i < COUNTOF(labels); ++i) {
		const Label &l = labels[i];
		p.setUniform(l.uniform, (float)l.x, (float)l.y, (float)l.w, (float)l.h);
	}
	*/

	return p;
}

int scenenum = 0;
int last_scene = 0;

static void drawIntro(float tick) {
	glEnable(GL_BLEND);
	renderPass(tick, framebuffer, prog_intro).compute();
	glDisable(GL_BLEND);
	renderPass(tick, screen, prog_blitter).compute();
}

void video_paint(float tick) {
#ifdef TOOL
SHADERS(POLL_SHADER_FILE)
#undef POLL_SHADER_FILE
#define POLL_PROGRAM(p, ...) prog_##p.poll();
PROGRAMS(POLL_PROGRAM)
#undef POLL_PROGRAM
#endif

	drawIntro(tick);
/*
	gt = tick;
	float t = tick;
	scenenum = 0;
#define SCENE(name, length) \
	if (t < length) { \
		draw##name(t); \
		last_scene = scenenum; \
		return; \
	} \
	t -= length; \
	++scenenum;

	SCENE(Intro, 2560);
	*/

	/*screen.bind();
	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	*/
}
