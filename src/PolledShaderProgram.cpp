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

	try {
		if (!vertex_) {
			program_ = Program::load(fragment_->sources());
			uniforms_ = fragment_->uniforms();
		} else {
			Program &&new_program = Program::load(fragment_->sources(), vertex_->sources());
			if (!new_program.valid())
				throw std::runtime_error("New program is not valid");
			shader::UniformsMap new_uniforms = vertex_->uniforms();
			appendUniforms(new_uniforms, fragment_->uniforms());
			program_ = std::move(new_program);
			uniforms_ = std::move(new_uniforms);
			MSG("Built program %u", program_.name());
		}
		return endUpdate();
	} catch (const std::runtime_error& e) {
		MSG("Error updating program: %s", e.what());
	}

	return false;
}
