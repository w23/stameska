#include "PolledShaderProgram.h"

PolledShaderProgram::PolledShaderProgram(Preprocessor preprocessor, const std::shared_ptr<PolledShaderSource> &fragment)
	: preprocessor_(preprocessor)
	, fragment_(fragment)
{
}

PolledShaderProgram::PolledShaderProgram(Preprocessor preprocessor, const std::shared_ptr<PolledShaderSource> &vertex, const std::shared_ptr<PolledShaderSource> &fragment)
	: preprocessor_(preprocessor)
	, vertex_(vertex)
	, fragment_(fragment)
{
}

bool PolledShaderProgram::poll(unsigned int poll_seq) {
	if (!beginUpdate(poll_seq))
		return false;

	bool need_update = fragment_->poll(poll_seq);
	need_update |= vertex_ && vertex_->poll(poll_seq);
	if (!need_update)
		return false;

	auto fragment = preprocessor_(fragment_->flatSource());
	if (!fragment) {
		MSG("Error preprocessing fragment shader: %s", fragment.error().c_str());
		return false;
	}

	auto vertex = preprocessor_(vertex_->flatSource());
	if (!vertex) {
		MSG("Error preprocessing vertex shader: %s", vertex.error().c_str());
		return false;
	}

	auto program_result = Program::create(fragment.value(), vertex.value());
	if (!program_result.hasValue()) {
		MSG("Error updating program: %s", program_result.error().c_str());
		return false;
	}

	shader::UniformsMap new_uniforms = vertex_->flatSource().uniforms();
	const auto result = appendUniforms(new_uniforms, fragment_->flatSource().uniforms());
	if (!result.hasValue()) {
		MSG("Cannot merge uniforms: %s", result.error().c_str());
		return false;
	}

	program_ = std::move(program_result).value();
	uniforms_ = std::move(new_uniforms);
	return endUpdate();
}
