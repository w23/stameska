#include "ShaderSource.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

int testShaderSource() {
	const auto t1 = shader::Source::load("simple");
	assert(t1.chunks().size() == 1);
	assert(t1.chunks()[0].type == shader::Source::Chunk::Type::String);
	assert(t1.chunks()[0].value == "simple");
	assert(t1.uniforms().empty());

	const auto t2 = shader::Source::load("sim#vec3 not ple");
	assert(t2.chunks().size() == 1);
	assert(t2.chunks()[0].type == shader::Source::Chunk::Type::String);
	assert(t2.chunks()[0].value == "simnot ple");
	assert(t2.uniforms().size() == 1);
	assert(t2.uniforms().find("not")->second.type == shader::UniformType::Vec3);

	const auto t3 = shader::Source::load("lol #include \"kek\"sim#vec3 not ple");
	assert(t3.chunks().size() == 3);
	assert(t3.chunks()[0].type == shader::Source::Chunk::Type::String);
	assert(t3.chunks()[0].value == "lol ");
	assert(t3.chunks()[1].type == shader::Source::Chunk::Type::Include);
	assert(t3.chunks()[1].value == "kek");
	assert(t3.chunks()[2].type == shader::Source::Chunk::Type::String);
	assert(t3.chunks()[2].value == "simnot ple");
	assert(t3.uniforms().size() == 1);
	assert(t3.uniforms().find("not")->second.type == shader::UniformType::Vec3);

	const auto t4 = shader::Source::load("lo#version 31337\nl #include \"kek\"sim#vec3 not ple");
	assert(t4.version() == 31337);
	assert(t4.chunks().size() == 3);
	assert(t4.chunks()[0].type == shader::Source::Chunk::Type::String);
	assert(t4.chunks()[0].value == "lo\nl ");
	assert(t4.chunks()[1].type == shader::Source::Chunk::Type::Include);
	assert(t4.chunks()[1].value == "kek");
	assert(t4.chunks()[2].type == shader::Source::Chunk::Type::String);
	assert(t4.chunks()[2].value == "simnot ple");
	assert(t4.uniforms().size() == 1);
	assert(t4.uniforms().find("not")->second.type == shader::UniformType::Vec3);

	return 0;
}
