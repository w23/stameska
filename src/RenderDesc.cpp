#include "RenderDesc.h"
#include "YamlParser.h"

#include <assert.h>
#include <algorithm>

namespace renderdesc {

static Texture::PixelType pixelTypeFromString(const std::string &s) {
	// TODO case insensitivity
	if (s == "RGBA8") return Texture::PixelType::RGBA8;
	if (s == "RGBA16F") return Texture::PixelType::RGBA16F;
	if (s == "RGBA32F") return Texture::PixelType::RGBA32F;
	//if (s == "Depth24") return Texture::PixelType::Depth24;
	throw std::runtime_error(format("Unexpected pixel type %s", s.c_str()));
}

static Command::Flag flagFromString(const std::string &s) {
}

class Loader {
	const yaml::Mapping &root_;
	Pipeline pipeline_;
	struct {
		std::vector<std::string> framebuffer;
		std::vector<std::string> texture;
		std::vector<std::string> program;
		std::vector<std::string> shader;
	} names_;

	int readVariable(const std::string &s) {
	}

	int getShaderIndex(const std::string &s) {
		const auto it = std::find(names_.shader.begin(), names_.shader.end(), s);
		if (it != names_.shader.end())
			return (int)(it - names_.shader.begin());

		const int ret = (int)names_.shader.size();
		names_.shader.push_back(s);
		return ret;
	}


	void loadFramebuffers() {
		// TODO should framebuffers be mapping not sequence?
		const yaml::Sequence &fbs = root_.getSequence("framebuffers");
		for (const auto &fb_value: fbs) {
			const yaml::Mapping &yfb = fb_value.getMapping();
			const std::string &name = yfb.getString("name");
			if (std::find(names_.framebuffer.begin(), names_.framebuffer.end(), name) != names_.framebuffer.end())
				throw std::runtime_error(format("Framebuffer '%s' is not unique", name.c_str()));

			const size_t framebuffer_index = names_.framebuffer.size();
			assert(framebuffer_index == pipeline_.framebuffers.size());

			const yaml::Sequence &size = yfb.getSequence("size");

			const int width = readVariable(size.at(0).getString());
			const int height = readVariable(size.at(1).getString());

			const yaml::Mapping &textures = yfb.getMapping("textures");
			Framebuffer fb;
			fb.textures_count = 0;
			for (const auto &[tex_name, tex_type_name]: textures.map()) {
				if (fb.textures_count >= MAX_TARGET_TEXTURES)
					throw std::runtime_error(format("Too many targets for framebuffer %s, max: %d",
						name.c_str(), MAX_TARGET_TEXTURES));

				const Texture::PixelType tex_type = pixelTypeFromString(tex_type_name.getString());

				const size_t tex_index = names_.texture.size();
				assert(tex_index == pipeline_.textures.size());

				names_.texture.emplace_back(name + "." + tex_name);
				pipeline_.textures.emplace_back(width, height, tex_type);

				fb.textures[fb.textures_count] = (int)tex_index;
				++fb.textures_count;
			}
	
			pipeline_.framebuffers.push_back(std::move(fb));
			names_.framebuffer.push_back(name);
		}
	}

	void loadPrograms() {
		for (const auto &ip: root_.getMapping("programs").map()) {
			const std::string &name = ip.first;

			const yaml::Mapping &yp = ip.second.getMapping();

			const int fragment_index = getShaderIndex(yp.getString("fragment"));
			const int vertex_index = getShaderIndex(yp.getString("vertex"));

			names_.program.emplace_back(name);
			pipeline_.programs.emplace_back(vertex_index, fragment_index);
		}
	}

	void loadCommands() {
		for (const auto &ycmd_value: root_.getSequence("paint")) {
			if (ycmd_value.isString() && ycmd_value.getString() == "drawFullscreen") {
				pipeline_.commands.emplace_back(Command::DrawFullscreen());
				continue;
			}

			const yaml::Mapping &ycmd = ycmd_value.getMapping();
			const auto it = ycmd.map().begin();
			const std::string &op = it->first;
			const yaml::Value &yargs = it->second;

			if (op == "bindFramebuffer") {
				const auto index = getFramebufferIndex(yargs.getString());
				pipeline_.commands.emplace_back(Command::BindFramebuffer(index));
			} else if (op == "useProgram") {
				const auto index = getProgramIndex(yargs.getString());
				pipeline_.commands.emplace_back(Command::UseProgram(index));
			} else if (op == "bindTexture") {
				for (const auto &[yuniform, ytexture]: yargs.getMapping().map()) {
					pipeline_.commands.emplace_back(
						Command::BindTexture(yuniform, getTextureIndex(ytexture.getString())));
				}
			} else if (op == "clear") {
				// FIXME read actual values
				pipeline_.commands.emplace_back(
					Command::Clear(0, 0, 0, 0, true));
			} else if (op == "enable") {
				const Command::Flag flag = flagFromString(yargs.getString());
				pipeline_.commands.emplace_back(Command::Enable(flag));
			} else if (op == "disable") {
				const Command::Flag flag = flagFromString(yargs.getString());
				pipeline_.commands.emplace_back(Command::Disable(flag));
			} else if (op == "drawArrays") {
				const yaml::Mapping &ymap = yargs.getMapping();
				pipeline_.commands.emplace_back(
					Command::DrawArrays(
						readVariable(ymap.getString("start")),
						readVariable(ymap.getString("count"))
					)
				);
			}
		}
	}

public:
	Loader(const yaml::Mapping &root) : root_(root) {}

	Pipeline load() {
		if (root_.hasKey("framebuffers"))
			loadFramebuffers();

		loadPrograms();

		loadCommands();

		return std::move(pipeline_);
	}
};

Pipeline Pipeline::loadFromString(std::string_view s) {
	Loader loader(yaml::parse(s).getMapping());
	return loader.load();
}

} // namespace renderdesc
