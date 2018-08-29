#include "video.h"

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#define NOMSG
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include "glext.h"

#include <stdint.h>

#define GL_FUNC_LIST(X) \
  X(PFNGLCREATESHADERPROGRAMVPROC, CreateShaderProgramv) \
  X(PFNGLUSEPROGRAMPROC, UseProgram) \
  X(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation) \
  X(PFNGLUNIFORM1IPROC, Uniform1i) \
  X(PFNGLUNIFORM1FPROC, Uniform1f) \
  X(PFNGLUNIFORM2FPROC, Uniform2f) \
  X(PFNGLUNIFORM4FPROC, Uniform4f) \
  X(PFNGLGENFRAMEBUFFERSPROC, GenFramebuffers) \
  X(PFNGLBINDFRAMEBUFFERPROC, BindFramebuffer) \
  X(PFNGLFRAMEBUFFERTEXTURE2DPROC, FramebufferTexture2D) \
  X(PFNGLDRAWBUFFERSPROC, DrawBuffers) \
  X(PFNGLACTIVETEXTUREPROC, ActiveTexture) \
  X(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog) \
  X(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog) \
  X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, CheckFramebufferStatus) \

#ifdef _WIN32
namespace {
#define GL_DECLARE_FUNC(type, name) type gl ## name;
	GL_FUNC_LIST(GL_DECLARE_FUNC)
#undef GL_DECLARE_FUNC
}
#endif

#ifdef TOOL
//#define DEBUG_GL
#endif

#ifndef DEBUG_GL
#define GL(f) f
#define glGetError()
#else
static const char *a__GlPrintError(int error) {
	const char *errstr = "UNKNOWN";
	switch (error) {
	case GL_INVALID_ENUM: errstr = "GL_INVALID_ENUM"; break;
	case GL_INVALID_VALUE: errstr = "GL_INVALID_VALUE"; break;
	case GL_INVALID_OPERATION: errstr = "GL_INVALID_OPERATION"; break;
#ifdef GL_STACK_OVERFLOW
	case GL_STACK_OVERFLOW: errstr = "GL_STACK_OVERFLOW"; break;
#endif
#ifdef GL_STACK_UNDERFLOW
	case GL_STACK_UNDERFLOW: errstr = "GL_STACK_UNDERFLOW"; break;
#endif
	case GL_OUT_OF_MEMORY: errstr = "GL_OUT_OF_MEMORY"; break;
#ifdef GL_TABLE_TOO_LARGE
	case GL_TABLE_TOO_LARGE: errstr = "GL_TABLE_TOO_LARGE"; break;
#endif
	case 1286: errstr = "INVALID FRAMEBUFFER"; break;
	};
	return errstr;
}
#define GL(f) \
	do { \
		f;\
		GLCHECK(#f); \
	} while(0)
static void GLCHECK(const char *func) {
	const int glerror = glGetError();
	if (glerror != GL_NO_ERROR) {
#ifdef _WIN32
		MessageBoxA(NULL, a__GlPrintError(glerror), func, 0);
		ExitProcess(0);
#else
		printf("%s\n", a__GlPrintError(glerror));
		abort();
#endif
	};
}
#endif /* DEBUG_GL */

#ifndef TOOL
#include "shaders.h"
#define MSG(str) MessageBoxA(NULL, str, "Error", MB_OK)
#else
#include "atto/app.h"
#define MSG(...) aAppDebugPrintf(__VA_ARGS__)

class PolledFile {
	const char *const filename_;
	unsigned int seq_ = 0;
	char buffer_[65536];

#ifdef _WIN32
	struct Metadata {
		size_t size = 0;
		FILETIME time = { 0, 0 };

		bool valid() const {
			return size != 0 && time.dwHighDateTime != 0 && time.dwLowDateTime != 0;
		}

		bool operator==(const Metadata& rhs) const {
			return size == rhs.size
				&& time.dwHighDateTime == rhs.time.dwHighDateTime
				&& time.dwLowDateTime == rhs.time.dwLowDateTime;
		}
	};
#else
	struct Metadata {
		size_t size = 0;
		struct timespec time = { 0, 0 };

		bool valid() const {
			return size != 0 && time.tv_sec != 0 && time.tv_nsec != 0;
		}

		bool operator==(const Metadata& rhs) const {
			return size == rhs.size
				&& time.tv_sec == rhs.time.tv_sec
				&& time.tv_nsec == rhs.time.tv_nsec;
		}
	};
#endif

	Metadata metadata_;

	static Metadata readFileMetadata(const char *filename) {
#ifdef _WIN32
		Metadata metadata;
		const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fh == INVALID_HANDLE_VALUE) {
			MSG("Cannot check file '%s' modification time: %x", filename, GetLastError());
			return metadata;
		}

		LARGE_INTEGER splurge_integer;
		if (!GetFileSizeEx(fh, &splurge_integer) || splurge_integer.QuadPart > 1024 * 1024) {
			MSG("Cannot get file '%s' size or size is too large", filename);
			goto exit;
		}

		FILETIME ft;
		if (!GetFileTime(fh, NULL, NULL, &ft)) {
			MSG("Cannot get file '%s' modification time", filename);
			goto exit;
		}

		metadata.size = splurge_integer.QuadPart;
		metadata.time = ft;

	exit:
		CloseHandle(fh);
		return metadata;
#else
	struct stat st;
	stat(filename, &st);
	Metadata metadata;
	metadata.size = st.st_size;
	metadata.time = st.st_mtim;
	return metadata;
#endif
	}

	static int readFileContents(const char *filename, char *buffer, size_t max_size) {
#ifdef _WIN32
		const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fh == INVALID_HANDLE_VALUE) {
			MSG("Cannot open file '%s'", filename);
			return 0;
		}

		LARGE_INTEGER splurge_integer;
		if (!GetFileSizeEx(fh, &splurge_integer) || splurge_integer.QuadPart > 1024 * 1024) {
			MSG("Cannot get file '%s' size or size is too large", filename);
			return 0;
		}

		const size_t size = splurge_integer.QuadPart;
		DWORD bytes_read = 0;
		if (size >= max_size || !ReadFile(fh, buffer, size, &bytes_read, NULL) || bytes_read != size) {
			MSG("Cannot read %d bytes from file '%s'", size, filename);
			bytes_read = 0;
		}
		buffer[bytes_read] = '\0';

		CloseHandle(fh);
		return bytes_read != 0;
#else
	const int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		MSG("Cannot open file %s", filename);
		return 0;
	}

	struct stat st;
	stat(filename, &st);

	if ((int)st.st_size >= (int)max_size || read(fd, buffer, st.st_size) != st.st_size) {
		MSG("Cannot read %d bytes from file '%s'", (int)st.st_size, filename);
		st.st_size = 0;
	}
	buffer[st.st_size] = '\0';

	close(fd);
	return st.st_size != 0;

#endif
	}

public:
	PolledFile(const char *filename) : filename_(filename) {}

	unsigned int seq() const { return seq_; }
	const char* data() const { return buffer_; }
	size_t size() const { return metadata_.size; }

	bool poll() {
		const Metadata new_metadata = readFileMetadata(filename_);
		if (!new_metadata.valid())
			return false;

		if (new_metadata == metadata_)
			return false;

		if (!readFileContents(filename_, buffer_, sizeof(buffer_)))
			return false;

		metadata_ = new_metadata;
		++seq_;
		return true;
	}
};
#endif

template <typename T>
class ConstArrayView {
	const T *ptr_;
	size_t size_;

public:
	ConstArrayView(const T& single) : ptr_(&single), size_(1) {}
	ConstArrayView(const T* ptr, size_t size) : ptr_(ptr), size_(size) {}

	size_t size() const { return size_; }
	const T *ptr() const { return ptr_; }
};


class Program {
protected:
	GLuint name = 0;

	static GLuint createFragmentProgram(const ConstArrayView<const char*>& sources) {
		return createSeparableProgram(GL_FRAGMENT_SHADER, sources);
	}

	static GLuint createSeparableProgram(GLenum type, const ConstArrayView<const char*>& sources) {
		const GLuint pid = glCreateShaderProgramv(type, sources.size(), sources.ptr());

#define SHADER_DEBUG
#ifdef SHADER_DEBUG
		{
			int result;
			char info[2048];
#ifdef _WIN32
#define glGetObjectParameterivARB ((PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress("glGetObjectParameterivARB"))
#define glGetInfoLogARB ((PFNGLGETINFOLOGARBPROC) wglGetProcAddress("glGetInfoLogARB"))
#endif
			glGetObjectParameterivARB(pid, GL_OBJECT_LINK_STATUS_ARB, &result);
			glGetInfoLogARB(pid, 2047, NULL, (char*)info);
			if (!result)
			{
				MSG(info);
				//MessageBoxA(NULL, info, "LINK", 0x00000000L);
#ifndef TOOL
				ExitProcess(0);
#endif
			}
		}
#endif

		return pid;
	}

public:
	void load(const ConstArrayView<const char*>& sources) {
		name = createFragmentProgram(sources);
	}

	bool use(int w, int h, float t) const {
		if (!name)
			return false;

		glUseProgram(name);
		glUniform1f(glGetUniformLocation(name, "t"), t);
		glUniform2f(glGetUniformLocation(name, "RES"), w, h);
		return true;
	}

	const Program& setUniform(const char *uname, int i) const {
		glUniform1i(glGetUniformLocation(name, uname), i);
		return *this;
	}

	const Program& setUniform(const char *uname, float f) const {
		glUniform1f(glGetUniformLocation(name, uname), f);
		return *this;
	}

	const Program& setUniform(const char *uname, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(name, uname), x, y, z, w);
		return *this;
	}

	void compute() const {
		glRects(-1, -1, 1, 1);
	}
};

class Texture {
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;

	friend class Framebuffer;
public:
	void init() { glGenTextures(1, &name_); }
	void upload(int w, int h, int comp, int type, const void *data) {
		w_ = w;
		h_ = h;
		GL(glBindTexture(GL_TEXTURE_2D, name_));
		GL(glTexImage2D(GL_TEXTURE_2D, 0, comp, w, h, 0, GL_RGBA, type, data));
		GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, data ? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, data ? GL_REPEAT : GL_CLAMP);
	}

	int bind(int slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, name_);
		return slot;
	}
};

class Framebuffer {
	GLuint name_ = 0;
	int w_ = 0, h_ = 0;
	int targets_ = 0;

	const static GLuint draw_buffers_[4];

public:
	int w() const { return w_; }
	int h() const { return h_; }

	void init(int w, int h) { w_ = w; h_ = h; targets_ = 1; }

	Framebuffer& init() {
		glGenFramebuffers(1, &name_);
		GL(glBindFramebuffer(GL_FRAMEBUFFER, name_));
		return *this;
	}

	Framebuffer &attach(const Texture &t) {
		GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + targets_, GL_TEXTURE_2D, t.name_, 0));
		w_ = t.w_;
		h_ = t.h_;
		++targets_;
		return *this;
	}

	void check() {
		const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
#ifdef _WIN32
			MessageBoxA(NULL, "Framebuffer is not ready", "", MB_OK);
			ExitProcess(0);
#else
			MSG("Framebuffer not ready");
#endif
		}
	}

	void bind() const {
		GL(glBindFramebuffer(GL_FRAMEBUFFER, name_));
		glViewport(0, 0, w_, h_);
		if (name_)
			glDrawBuffers(targets_, draw_buffers_);
	}
};

const GLuint Framebuffer::draw_buffers_[4] = {
	GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
};

#ifdef _WIN32
#define DEFAULT_FONT L"Tahoma"

struct Label {
	const char *uniform;
	const char *ansi;
	const wchar_t *unicode;
	const wchar_t *font;
	int size;
	int x, y, w, h;
};

const unsigned char greets_raw[720] = {
	0x61, 0x00, 0x6C, 0x00, 0x63, 0x00, 0x61, 0x00, 0x74, 0x00, 0x72, 0x00,
	0x61, 0x00, 0x7A, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x63, 0x00, 0x6F, 0x00,
	0x6E, 0x00, 0x73, 0x00, 0x63, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x75, 0x00,
	0x73, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x73, 0x00, 0x73, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x70, 0x00,
	0x69, 0x00, 0x72, 0x00, 0x61, 0x00, 0x63, 0x00, 0x79, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x63, 0x00, 0x74, 0x00, 0x72, 0x00, 0x6C, 0x00, 0x2D, 0x00,
	0x61, 0x00, 0x6C, 0x00, 0x74, 0x00, 0x2D, 0x00, 0x74, 0x00, 0x65, 0x00,
	0x73, 0x00, 0x74, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x64, 0x00, 0x61, 0x00,
	0x72, 0x00, 0x6B, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x74, 0x00, 0x65, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x78, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x66, 0x00, 0x61, 0x00, 0x69, 0x00, 0x72, 0x00,
	0x6C, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x66, 0x00, 0x61, 0x00, 0x72, 0x00, 0x62, 0x00, 0x72, 0x00,
	0x61, 0x00, 0x75, 0x00, 0x73, 0x00, 0x63, 0x00, 0x68, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x66, 0x00, 0x6C, 0x00, 0x31, 0x00, 0x6E, 0x00, 0x65, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x66, 0x00, 0x6D, 0x00, 0x73, 0x00, 0x5F, 0x00,
	0x63, 0x00, 0x61, 0x00, 0x74, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x66, 0x00,
	0x72, 0x00, 0x61, 0x00, 0x67, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x6B, 0x00,
	0x61, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x20, 0x00,
	0x63, 0x00, 0x79, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x64, 0x00,
	0x65, 0x00, 0x20, 0x00, 0x67, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x75, 0x00,
	0x70, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x78, 0x00,
	0x61, 0x00, 0x6C, 0x00, 0x6F, 0x00, 0x66, 0x00, 0x66, 0x00, 0x6C, 0x00,
	0x65, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x72, 0x00,
	0x63, 0x00, 0x75, 0x00, 0x72, 0x00, 0x79, 0x00, 0x0D, 0x00, 0x0A, 0x00,
	0x6E, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x6F, 0x00, 0x69, 0x00, 0x6C, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6E, 0x00,
	0x67, 0x00, 0x65, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x6F, 0x00, 0x72, 0x00,
	0x62, 0x00, 0x69, 0x00, 0x74, 0x00, 0x61, 0x00, 0x6C, 0x00, 0x64, 0x00,
	0x65, 0x00, 0x63, 0x00, 0x61, 0x00, 0x79, 0x00, 0x0D, 0x00, 0x0A, 0x00,
	0x70, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x78, 0x00, 0x69, 0x00, 0x75, 0x00,
	0x6D, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x70, 0x00, 0x72, 0x00, 0x69, 0x00,
	0x73, 0x00, 0x6D, 0x00, 0x62, 0x00, 0x65, 0x00, 0x69, 0x00, 0x6E, 0x00,
	0x67, 0x00, 0x73, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x71, 0x00, 0x75, 0x00,
	0x69, 0x00, 0x74, 0x00, 0x65, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0xED, 0x30,
	0xB8, 0x30, 0xB3, 0x30, 0xDE, 0x30, 0x0D, 0x00, 0x0A, 0x00, 0x73, 0x00,
	0x61, 0x00, 0x6E, 0x00, 0x64, 0x00, 0x73, 0x00, 0x0D, 0x00, 0x0A, 0x00,
	0x73, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x65, 0x00, 0x6E, 0x00,
	0x73, 0x00, 0x74, 0x00, 0x61, 0x00, 0x68, 0x00, 0x6C, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x73, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x6C, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x73, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00,
	0x65, 0x00, 0x6D, 0x00, 0x6B, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x74, 0x00,
	0x2D, 0x00, 0x72, 0x00, 0x65, 0x00, 0x78, 0x00, 0x0D, 0x00, 0x0A, 0x00,
	0x74, 0x00, 0x68, 0x00, 0x65, 0x00, 0x20, 0x00, 0x6E, 0x00, 0x65, 0x00,
	0x70, 0x00, 0x68, 0x00, 0x65, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x6D, 0x00,
	0x73, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x74, 0x00, 0x68, 0x00, 0x72, 0x00,
	0x6F, 0x00, 0x62, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x74, 0x00, 0x69, 0x00,
	0x74, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x74, 0x00,
	0x6F, 0x00, 0x6D, 0x00, 0x6F, 0x00, 0x68, 0x00, 0x69, 0x00, 0x72, 0x00,
	0x6F, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x76, 0x00, 0x61, 0x00, 0x61, 0x00,
	0x68, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00, 0x61, 0x00, 0x0D, 0x00,
	0x0A, 0x00, 0x76, 0x00, 0x69, 0x00, 0x72, 0x00, 0x74, 0x00, 0x75, 0x00,
	0x61, 0x00, 0x6C, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6C, 0x00, 0x6C, 0x00,
	0x75, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x73, 0x00,
	0x0D, 0x00, 0x0A, 0x00, 0x79, 0x00, 0x6F, 0x00, 0x75, 0x00, 0x74, 0x00,
	0x68, 0x00, 0x20, 0x00, 0x75, 0x00, 0x70, 0x00, 0x72, 0x00, 0x69, 0x00,
	0x73, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x0D, 0x00, 0x0A, 0x00,
	0x66, 0x00, 0x75, 0x00, 0x74, 0x00, 0x75, 0x00, 0x72, 0x00, 0x65, 0x00,
	0x20, 0x00, 0x63, 0x00, 0x72, 0x00, 0x65, 0x00, 0x77, 0x00, 0x00, 0x00
};

unsigned char endscreen[120] = {
	0x6a, 0x00, 0x65, 0x00, 0x74, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x67, 0x00, 0x7C,
	0x00, 0x3D, 0x04, 0x30, 0x04, 0x37, 0x04, 0x32, 0x04, 0x30, 0x04, 0x3D,
	0x04, 0x38, 0x04, 0x35, 0x04, 0x0D, 0x00, 0x0A, 0x00, 0x0D, 0x00, 0x0A,
	0x00, 0x6B, 0x00, 0x65, 0x00, 0x65, 0x00, 0x6E, 0x00, 0x7C, 0x00, 0x6D,
	0x00, 0x75, 0x00, 0x73, 0x00, 0x69, 0x00, 0x63, 0x00, 0x0D, 0x00, 0x0A,
	0x00, 0x70, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x76, 0x00, 0x6F, 0x00, 0x64,
	0x00, 0x7C, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x65, 0x00, 0x0D,
	0x00, 0x0A, 0x00, 0x0D, 0x00, 0x0A, 0x00, 0x65, 0x00, 0x76, 0x00, 0x6F,
	0x00, 0x6B, 0x00, 0x65, 0x00, 0x7C, 0x00, 0x32, 0x00, 0x30, 0x00, 0x31,
	0x00, 0x38, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x38, 0x00, 0x00, 0x00,
};


//const wchar_t rojikoma[] = { 0x30ed, 0x30b8, 0x30b3, 0x30de, 0x0020, 0x0425, 0x0423, 0x0419, 0 };

Label labels[] = {
	{"lab_greets", NULL, (const wchar_t*)greets_raw, L"Verdana", 12, 0, 0, 0, 0},
	//{"lab_jetlag", "jetlag", NULL, L"Tahoma", 144, 0, 0, 0, 0},
	{"lab_title", "nazvanie", NULL, L"Tahoma", 144, 0, 0, 0, 0},
	//{"lab_rojikoma", NULL, rojikoma, L"Verdana", 42, 0, 0, 0, 0},
	{"lab_endscreen", /*"jetlag|...\n\nkeen|music\nprovod|code\n\nevoke|2018.08"*/NULL, (const wchar_t*)endscreen, L"Tahoma", 12, 0, 0, 0, 0},
	{"lab_source", NULL, NULL, L"Courier New", 12, 0, 0, 0, 0},
};

static void initText(Texture &text) {
	const unsigned int TEXT_WIDTH = 1024, TEXT_HEIGHT = 4096;
	const HDC text_dc = CreateCompatibleDC(NULL);
	BITMAPINFO bitmap_info = { 0 };
	bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmap_info.bmiHeader.biWidth = TEXT_WIDTH;
	bitmap_info.bmiHeader.biHeight = TEXT_HEIGHT;
	bitmap_info.bmiHeader.biBitCount = 32;
	bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biPlanes = 1;
	void *bitmap_ptr = NULL;
	const HBITMAP dib = CreateDIBSection(text_dc, &bitmap_info, DIB_RGB_COLORS, &bitmap_ptr, NULL, 0);
	const HGDIOBJ obj = SelectObject(text_dc, dib);
	RECT rect = { 0, 0, TEXT_WIDTH, TEXT_HEIGHT };
	SetTextColor(text_dc, RGB(255, 255, 255));
	SetBkMode(text_dc, TRANSPARENT);

	int size = 0;
	HFONT font = 0;
	for (int i = 0; i < COUNTOF(labels); ++i) {
		Label &l = labels[i];

		font = CreateFont(l.size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, l.font);
		SelectObject(text_dc, font);

		// needs rect; keeps newlines
		if (l.ansi) {
			DrawTextA(text_dc, l.ansi, -1, &rect, DT_CALCRECT);
			DrawTextA(text_dc, l.ansi, -1, &rect, 0);
			// DT_SINGLELINE | DT_NOCLIP);
		} else {
			DrawTextW(text_dc, l.unicode, -1, &rect, DT_CALCRECT);
			DrawTextW(text_dc, l.unicode, -1, &rect, 0);
			// DT_SINGLELINE | DT_NOCLIP);
		}

		l.x = rect.left;
		l.y = rect.top;
		l.w = rect.right - rect.left;
		l.h = rect.bottom - rect.top;

#ifdef TOOL
		MSG("%s %d %d %d %d", l.uniform, l.x, l.y, l.w, l.h);
#endif

		rect.left = 0;
		rect.right = TEXT_WIDTH;
		rect.top = rect.bottom;
		rect.bottom = TEXT_HEIGHT;

		/*
		// ignores newlines
		TextOutW(text_dc, 0, 400, ro, 8);
		TextOutA(text_dc, 0, 600, shader_program, strlen(shader_program));
		*/
		DeleteObject(font);
	}

	text.upload(TEXT_WIDTH, TEXT_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_ptr);

	DeleteObject(dib);
	DeleteDC(text_dc);
}
#endif // WIN32

#define PCG32_INITIALIZER { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }
typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;
uint32_t pcg32_random_r(pcg32_random_t* rng)
{
	uint64_t oldstate = rng->state;
	// Advance internal state
	rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

#define NOISE_SIZE 1024
uint32_t noise_buffer[NOISE_SIZE * NOISE_SIZE];
static void initNoise(Texture &noise) {
	pcg32_random_t rand = PCG32_INITIALIZER;
	for (int i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i) {
		noise_buffer[i] = pcg32_random_r(&rand);
	}

	noise.upload(NOISE_SIZE, NOISE_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, noise_buffer);
}

#ifdef TOOL
class PollAdaptor {
	const PolledFile &file_;
	unsigned int seq_;

public:
	PollAdaptor(const PolledFile &file) : file_(file), seq_(file_.seq() - 1) {}

	const PolledFile &file() const { return file_; }

	bool consume() {
		if (file_.seq() != seq_) {
			seq_ = file_.seq();
			return true;
		}

		return false;
	}
};

template <int N>
class PolledProgram : public Program {
	PollAdaptor* const adaptors_;
	const char *sources[N];

public:
	PolledProgram(PollAdaptor *adaptors)
		: adaptors_(adaptors)
	{
		for (int i = 0; i < N; ++i)
			sources[i] = adaptors_[i].file().data();
	}

	void poll() {
		bool updated = false;
		for (int i = 0; i < N; ++i)
			updated |= adaptors_[i].consume();

		if (updated)
			load(ConstArrayView<const char*>(sources, N));
	}
};

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
Texture noise;
Texture frame;

Framebuffer screen;
Framebuffer framebuffer;

int w, h;
void video_init(int W, int H) {
	w = W; h = H;

#ifdef _WIN32
#define GL_LOAD_FUNC(type, name) gl ## name = (type)wglGetProcAddress("gl" # name);
	GL_FUNC_LIST(GL_LOAD_FUNC)
#undef GL_LOAD_FUNC
#endif

	//text.init();
	noise.init();
	frame.init();
	frame.upload(640, 480, GL_RGBA16F, GL_FLOAT, nullptr);
	screen.init(w, h);
	framebuffer.init().attach(frame).check();

	initNoise(noise);

#ifdef TOOL
#define POLL_SHADER_FILE(s) s.poll();
SHADERS(POLL_SHADER_FILE)
#undef INIT_PROGRAM

#else
#define LOAD_PROGRAM(p, ...) prog_##p.load(ConstArrayView<const char*>(p##_sources, COUNTOF(p##_sources)));
PROGRAMS(LOAD_PROGRAM)
#undef LOAD_PROGRAM
#endif

/*
#ifndef TOOL
	labels[COUNTOF(labels)-1].ansi = greetings;
#else
	labels[COUNTOF(labels)-1].ansi = greetings.data();
#endif

	initText(text);
*/
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static float gt;

static const Program& renderPass(float tick, const Framebuffer& fb, const Program& p) {
	fb.bind();

	int tslot = 0;
	p.use(fb.w(), fb.h(), tick);
	p.setUniform("N", noise.bind(tslot++));
	p.setUniform("F", frame.bind(tslot++));

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

	/*screen.bind();
	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	*/
}
