#pragma once

#include "PolledShaderSource.h"
#include "PolledResource.h"
#include "Program.h"

class PolledShaderProgram : public PolledResource {
public:
	PolledShaderProgram(const std::shared_ptr<PolledShaderSource> &fragment);

	PolledShaderProgram(const std::shared_ptr<PolledShaderSource> &vertex, const std::shared_ptr<PolledShaderSource> &fragment);

	bool poll(unsigned int poll_seq);

	const Program& get() const { return program_; }
	const Program& operator->() const { return program_; }

	const shader::UniformsMap& uniforms() const { return uniforms_; }

private:
	const std::shared_ptr<PolledShaderSource> vertex_, fragment_;
	shader::UniformsMap uniforms_;
	Program program_;
};
