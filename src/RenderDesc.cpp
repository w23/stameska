#include "RenderDesc.h"
#include "YamlParser.h"

#include <assert.h>
#include <algorithm>

namespace renderdesc {

static Texture::PixelType pixelTypeFromString(const std::string &s) {
	if (s == "RGBA8") return Texture::PixelType::RGBA8;
	if (s == "RGBA16F") return Texture::PixelType::RGBA16F;
	if (s == "RGBA32F") return Texture::PixelType::RGBA32F;
	//if (s == "Depth24") return Texture::PixelType::Depth24;
	throw std::runtime_error(format("Unexpected pixel type %s", s.c_str()));
}

static Command::Flag flagFromString(const std::string &s) {
	if (s == "DepthTest") return Command::Flag::DepthTest;
	if (s == "VertexProgramPointSize") return Command::Flag::VertexProgramPointSize;

	throw std::runtime_error(format("Unknown flag %s", s.c_str()));
}

class Loader {
	Pipeline &pipeline_;
	const yaml::Mapping &root_;
	struct {
		std::vector<std::string> framebuffer;
		std::vector<std::string> texture;
		std::vector<std::string> program;
		std::vector<std::string> shader;
	} names_;
	struct {
		std::vector<Command::Index> framebuffer;
		std::vector<Command::Index> texture;
	} indexes_;

	int readVariable(const std::string &s) {
		// FIXME read actual variable
		return intFromString(s);
	}

	Command::Index getFramebufferIndex(const std::string &s) {
		if (s == "SCREEN")
			return -1;

		const auto it = std::find(names_.framebuffer.begin(), names_.framebuffer.end(), s);
		if (it == names_.framebuffer.end())
			throw std::runtime_error(format("Unknown framebuffer %s", s.c_str()));

		return indexes_.framebuffer[it - names_.framebuffer.begin()];
	}

	Command::Index getProgramIndex(const std::string &s) {
		const auto it = std::find(names_.program.begin(), names_.program.end(), s);
		if (it == names_.program.end())
			throw std::runtime_error(format("Unknown program %s", s.c_str()));

		return indexes_.framebuffer[it - names_.program.begin()];
	}

	Command::Index getTextureIndex(const std::string &s) {
		const auto it = std::find(names_.texture.begin(), names_.texture.end(), s);
		if (it == names_.texture.end())
			throw std::runtime_error(format("Unknown texture %s", s.c_str()));

		return (int)(it - names_.texture.begin());
	}

	int getShaderIndex(const std::string &s) {
		const auto it = std::find(names_.shader.begin(), names_.shader.end(), s);
		if (it != names_.shader.end())
			return (int)(it - names_.shader.begin());

		const int ret = (int)names_.shader.size();
		names_.shader.push_back(s);
		pipeline_.shader_filenames.push_back(s);
		return ret;
	}

	void addTexture(Command::Index::Pingpong pp, const std::string &name, int width, int height, Texture::PixelType tex_type) {
		const int index = (int)names_.texture.size();
		names_.texture.emplace_back(name);
		pipeline_.textures.emplace_back(width, height, tex_type);
		indexes_.texture.emplace_back(Command::Index(index, pp));
	}

	void loadFramebuffers() {
		const yaml::Mapping &yfbs = root_.getMapping("framebuffers");
		for (const auto &[name, yfbv]: yfbs.map()) {
			if (std::find(names_.framebuffer.begin(), names_.framebuffer.end(), name) != names_.framebuffer.end())
				throw std::runtime_error(format("Framebuffer '%s' is not unique", name.c_str()));

			const yaml::Mapping &yfb = yfbv.getMapping();

			const size_t framebuffer_index = names_.framebuffer.size();
			assert(framebuffer_index == pipeline_.framebuffers.size());

			const yaml::Sequence &size = yfb.getSequence("size");

			const bool pingpong = yfb.hasKey("pingpong") && yfb.getInt("pingpong");

			const int width = readVariable(size.at(0).getString());
			const int height = readVariable(size.at(1).getString());

			const yaml::Sequence &ytextures = yfb.getSequence("textures");
			Framebuffer fb;
			fb.textures_count = 0;
			for (const auto &ytex: ytextures) {
				const yaml::Mapping &tex = ytex.getMapping();
				if (tex.map().size() != 1)
					throw std::runtime_error("Framebuffer target texture should have only one key:value pair: name and format");

				const auto &[tex_name, tex_type_name] = *tex.map().begin();
				if (fb.textures_count >= MAX_TARGET_TEXTURES)
					throw std::runtime_error(format("Too many targets for framebuffer %s, max: %d",
						name.c_str(), MAX_TARGET_TEXTURES));

				const Texture::PixelType tex_type = pixelTypeFromString(tex_type_name.getString());

				const size_t tex_index = names_.texture.size();
				assert(tex_index == pipeline_.textures.size());

				if (pingpong) {
					addTexture(Command::Index::Pingpong::Ping, name + "@ping." + tex_name, width, height, tex_type);
					addTexture(Command::Index::Pingpong::Pong, name + "@pong." + tex_name, width, height, tex_type);
				} else {
					addTexture(Command::Index::Pingpong::Fixed, name + "." + tex_name, width, height, tex_type);
				}

				fb.textures[fb.textures_count] = (int)tex_index;
				++fb.textures_count;
			}

			if (pingpong) {
				{
					const int index = (int)pipeline_.framebuffers.size();
					pipeline_.framebuffers.push_back(fb);
					names_.framebuffer.push_back(name + "@ping");
					indexes_.framebuffer.emplace_back(Command::Index(index, Command::Index::Pingpong::Ping));
				}

				for (int i = 0; i < fb.textures_count; ++i)
					fb.textures[i] += 1;
				{
					const int index = (int)pipeline_.framebuffers.size();
					pipeline_.framebuffers.push_back(fb);
					names_.framebuffer.push_back(name + "@pong");
					indexes_.framebuffer.emplace_back(Command::Index(index, Command::Index::Pingpong::Pong));
				}
			} else {
				const int index = (int)pipeline_.framebuffers.size();
				pipeline_.framebuffers.push_back(fb);
				names_.framebuffer.push_back(name);
				indexes_.framebuffer.emplace_back(Command::Index(index));
			}
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
			} else
				throw std::runtime_error(format("Unknown command %s", op.c_str()));
		}
	}

public:
	Loader(Pipeline &pipeline, const yaml::Mapping &root)
		: pipeline_(pipeline)
		, root_(root)
	{
	}

	void load() {
		if (root_.hasKey("framebuffers"))
			loadFramebuffers();

		loadPrograms();

		loadCommands();
	}
};

Pipeline::Pipeline(std::string_view s) {
	const auto root = yaml::parse(s);
	Loader loader(*this, root.getMapping());
	loader.load();
}

} // namespace renderdesc
