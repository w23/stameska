#include "PolledShaderProgram.h"

PolledShaderProgram::PolledShaderProgram(const std::shared_ptr<PolledShaderSource> &fragment)
	: fragment_(fragment)
{
}

PolledShaderProgram::PolledShaderProgram(const std::shared_ptr<PolledShaderSource> &vertex, const std::shared_ptr<PolledShaderSource> &fragment)
	: vertex_(vertex)
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

	auto program_result = Program::create(fragment_->sources(), vertex_->sources());
	if (!program_result.hasValue()) {
		MSG("Error updating program: %s", program_result.error().c_str());
		return false;
	}

	shader::UniformsMap new_uniforms = vertex_->uniforms();
	const auto result = appendUniforms(new_uniforms, fragment_->uniforms());
	if (!result.hasValue()) {
		MSG("Cannot merge uniforms: %s", result.error().c_str());
		return false;
	}

	program_ = std::move(program_result).value();
	uniforms_ = std::move(new_uniforms);
	return endUpdate();

	return false;
}
