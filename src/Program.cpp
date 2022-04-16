#include "Program.h"
#include "format.h"

namespace {
class Shader {
public:
	Shader(const Shader&) = delete;
	Shader(Shader &&other) : name_(other.name_) { other.name_ = 0; }

	static Expected<Shader,std::string> create(GLuint kind, const std::string& src) {
		GLuint name = glCreateShader(kind);
		GLchar const * const c_src = src.c_str();
		glShaderSource(name, 1, (const GLchar**)&c_src, nullptr);
		glCompileShader(name);

		GLint compiled = GL_FALSE;
		glGetShaderiv(name, GL_COMPILE_STATUS, &compiled);
		if (compiled != GL_TRUE) {
			GLsizei length = 0;
			char info[2048];
			glGetShaderInfoLog(name, COUNTOF(info), &length, info);
			glDeleteShader(name);

			std::string error_string = info;

			const int context = 3;
			int mod, line, pos;
			if (3 == sscanf(info, "%d:%d(%d)", &mod, &line, &pos)) {
				std::string_view sv(src);
				for (int i = 1; !sv.empty() && i < line + context; ++i) {
					size_t line_end = sv.find("\n");
					int add = 0;

					if (line_end == std::string::npos)
						line_end = sv.size();
					else add = 1;

					if (i >= line - context) {
						error_string += format("\n%d: ", i);
						error_string.append(sv.begin(), sv.begin() + line_end);
					}

					sv.remove_prefix(line_end + add);
				}
			} else {
				error_string = src;
			}

			return Unexpected(format("Shader compilation error: %s", error_string.c_str()));
		}

		return Shader(name);
	}

	~Shader() {
		if (name_ != 0)
			glDeleteShader(name_);
	}

	GLuint name() const { return name_; }

private:
	Shader(GLuint name) : name_(name) {}
	GLuint name_;
};

} // unnamed namespace

Program::Handle::Handle(Handle &&other) : name_(other.name_) { other.name_ = 0; }

Program::Handle& Program::Handle::operator=(Handle &&rhs) {
	if (name_)
		glDeleteProgram(name_);
	name_ = rhs.name_;
	rhs.name_ = 0;
	return *this;
}

Program::Handle::~Handle() {
	if (name_)
		glDeleteProgram(name_);
}

Expected<Program,std::string> Program::create(const std::string& fragment_src, const std::string &vertex_src) {
	Handle handle(glCreateProgram());
	if (!handle.name())
		return Unexpected(std::string("Cannot create program"));

	if (vertex_src.empty() || fragment_src.empty())
		return Unexpected(std::string("Empty shader sources"));

	auto vertex_result = Shader::create(GL_VERTEX_SHADER, vertex_src);
	if (!vertex_result.hasValue())
		return Unexpected("Cannot compile vertex shader: " + vertex_result.error());

	auto fragment_result = Shader::create(GL_FRAGMENT_SHADER, fragment_src);
	if (!fragment_result.hasValue())
		return Unexpected("Cannot compile fragment shader: " + fragment_result.error());

	glAttachShader(handle.name(), vertex_result->name());
	glAttachShader(handle.name(), fragment_result->name());

	glLinkProgram(handle.name());

	GLint linked = GL_FALSE;
	glGetProgramiv(handle.name(), GL_LINK_STATUS, &linked);
	if (linked != GL_TRUE) {
		GLsizei length = 0;
		char info[2048];
		glGetProgramInfoLog(handle.name(), COUNTOF(info), &length, info);
		return Unexpected(format("Program link error: %s", info));
	}

	MSG("Built program %u", handle.name());
	return Program(std::move(handle));
}

/*
static Handle createFragmentProgram(const std::string& source) {
	return createSeparableProgram(GL_FRAGMENT_SHADER, source.c_str());
}

static Handle createSeparableProgram(GLenum type, const char* sources) {
	Handle handle = Handle(glCreateShaderProgramv(type, 1, &sources));

	if (!handle.valid())
		return Handle();

	{
		int result;
		char info[2048];
		glGetObjectParameterivARB(handle.name(), GL_OBJECT_LINK_STATUS_ARB, &result);
		glGetInfoLogARB(handle.name(), 2047, NULL, (char*)info);
		if (!result)
		{
			MSG("Shader error: %s", info);
			return Handle();
		}
	}

	return handle;
}
*/

const Program &Program::use() const {
	glUseProgram(handle_.name());
	return *this;
}
