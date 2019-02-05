#define GL_GLEXT_PROTOTYPES
#include "sync.h"
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

struct sync_device *rocket_device;

struct RocketTrack {
	const char *name;
	int size;
	const char *data;
	int pos;
};

static GLuint current_program = 0;

#define COUNTOF(a) (sizeof(a)/sizeof(*(a)))

#include "stameska_export.c"

void *rocket_open(const char *filename, const char *mode) {
	for (int i = 0; i < COUNTOF(rocket_tracks); ++i) {
		struct RocketTrack *t = rocket_tracks + i;
		if (strcmp(filename, t->name) == 0) {
			t->pos = 0;
			return t;
		}
	}
	return NULL;
}

size_t rocket_read(void *ptr, size_t size, size_t nitems, void *stream) {
	struct RocketTrack *t = (struct RocketTrack*)stream;
	int to_read = (int)size * (int)nitems;
	const int size_left = t->size - t->pos;
	if (to_read > size_left)
		to_read = size_left;
	memcpy(ptr, t->data + t->pos, to_read);
	t->pos += to_read;
	return to_read;
}

int rocket_close(void *stream) {
	(void)stream;
	return 0;
}

struct sync_io_cb rocket_cb = { rocket_open, rocket_read, rocket_close };

void main() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_OPENGL | FULLSCREEN);

	rocket_device = sync_create_device("sync");
	sync_set_io_cb(rocket_device, &rocket_cb);

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

	sync_destroy_device(rocket_device);
}
