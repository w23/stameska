#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <SDL.h>

#define WIDTH 1280
#define HEIGHT 720
#define FULLSCREEN 0

#pragma code_seg(".textureInit")
static void textureInit(GLuint tex, int w, int h, int comp, int type) {//, void *data = NULL) {
	void *data = NULL;
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	*/
}

static GLuint current_program = 0;
#pragma code_seg(".useProgram")
static void useProgram(GLuint p, float t) {
	current_program = p;
	glUseProgram(p);
	// FIXME
	glUniform2f(glGetUniformLocation(p, "R"), 1280, 720);
	glUniform1f(glGetUniformLocation(p, "t"), t);
}

static GLuint programInit(const char *vertex, const char *fragment) {
	const GLuint pid = glCreateProgram();
	const GLuint fsid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsid, 1, &fragment, 0);
	glCompileShader(fsid);
	glAttachShader(pid, fsid);
	const GLuint vsid = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsid, 1, &vertex, 0);
	glCompileShader(vsid);
	glAttachShader(pid, vsid);
	glLinkProgram(pid);
	return pid;
}

#define MAX_PASS_TEXTURES 4
static const GLuint draw_buffers[MAX_PASS_TEXTURES] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};

#include "stameska_export.c"

void main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_OPENGL | FULLSCREEN);

	videoInit();

	const uint32_t start = SDL_GetTicks();
	for(;;) {
		const uint32_t now = SDL_GetTicks() - start;

		SDL_Event e;
		SDL_PollEvent(&e);
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
			break;

		videoPaint(now / 100.f);
		SDL_GL_SwapBuffers();
	}
}
