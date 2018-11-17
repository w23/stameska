#include "ShaderSource.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

int main() {
	const auto t1 = ShaderSource::create("simple");
	assert(std::get<ShaderSource::Shared>(t1)->source() == "simple");

	const auto t2 = ShaderSource::create("sim$(vec3 not)ple");
	assert(std::get<ShaderSource::Shared>(t2)->source() == "simnotple");
	assert(std::get<ShaderSource::Shared>(t2)->uniforms().find("not")->second.type == UniformType::Vec3);

	return 0;
}
