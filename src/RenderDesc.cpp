#include "RenderDesc.h"
#include "YamlParser.h"

#include <assert.h>
#include <algorithm>

namespace renderdesc {

static PixelType pixelTypeFromString(const std::string &s) {
	if (s == "RGBA8") return PixelType::RGBA8;
	if (s == "RGBA16F") return PixelType::RGBA16F;
	if (s == "RGBA32F") return PixelType::RGBA32F;
	//if (s == "Depth24") return PixelType::Depth24;
	throw std::runtime_error(format("Unexpected pixel type %s", s.c_str()));
}

static Command::Flag flagFromString(const std::string &s) {
	if (s == "DepthTest") return Command::Flag::DepthTest;
	if (s == "VertexProgramPointSize") return Command::Flag::VertexProgramPointSize;

	throw std::runtime_error(format("Unknown flag %s", s.c_str()));
}

static Command::DrawArrays::Mode drawModeFromString(const std::string &s) {
	if (s == "Points") return Command::DrawArrays::Mode::Points;
	if (s == "Triangles") return Command::DrawArrays::Mode::Triangles;
	if (s == "TriangleStrip") return Command::DrawArrays::Mode::TriangleStrip;
	if (s == "TriangleFan") return Command::DrawArrays::Mode::TriangleFan;
	if (s == "Lines") return Command::DrawArrays::Mode::Lines;
	if (s == "LineStrip") return Command::DrawArrays::Mode::LineStrip;
	if (s == "LineLoop") return Command::DrawArrays::Mode::LineLoop;

	throw std::runtime_error(format("Unknown mode %s", s.c_str()));
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

		return it - names_.program.begin();
	}

	Command::Index getTextureIndex(const std::string &s) {
		const auto it = std::find(names_.texture.begin(), names_.texture.end(), s);
		if (it == names_.texture.end())
			throw std::runtime_error(format("Unknown texture %s", s.c_str()));

		return indexes_.texture[it - names_.texture.begin()];
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

	void addTexture(const int index, Command::Index::Pingpong pp, const std::string &name, int width, int height, PixelType tex_type) {
		names_.texture.emplace_back(name);
		pipeline_.textures.emplace_back(width, height, tex_type);
		indexes_.texture.emplace_back(Command::Index(index, pp));
	}

	Expected<void, std::string> loadFramebuffers() {
		auto map_fb_result = root_.getMapping("framebuffers");
		if (!map_fb_result)
			return Unexpected(map_fb_result.error());

		for (const auto &[name, yfbv]: map_fb_result.value().get().map()) {
			if (std::find(names_.framebuffer.begin(), names_.framebuffer.end(), name) != names_.framebuffer.end())
				return Unexpected(format("Framebuffer '%s' is not unique", name.c_str()));

			auto yfb_result = yfbv.getMapping();
			if (!yfb_result)
				return Unexpected("Framebuffer " + name + " desc object is not a mapping");
			const yaml::Mapping &yfb = yfb_result.value();

			const size_t framebuffer_index = names_.framebuffer.size();
			assert(framebuffer_index == pipeline_.framebuffers.size());

			auto size_result = yfb.getSequence("size");
			if (!size_result)
				return Unexpected("Cannot read size for framebuffer " + name + ": " + size_result.error());
			const yaml::Sequence &size = size_result.value();

			const bool pingpong = yfb.hasKey("pingpong") && yfb.getInt("pingpong");

			const int width = readVariable(size.at(0).getString());
			const int height = readVariable(size.at(1).getString());

			auto textures_result = yfb.getSequence("textures");
			if (!textures_result)
				return Unexpected("Cannot read target textures for framebuffer " + name + ": " + textures_result.error());
			const yaml::Sequence &ytextures = textures_result.value();
			Framebuffer fb;
			fb.textures_count = 0;
			for (const auto &ytex: ytextures) {
				auto tex_result = ytex.getMapping();
				if (!tex_result)
					return Unexpected(format("%dth texture of framebuffer %s is not a mapping", fb.textures_count, name.c_str()));
				const yaml::Mapping &tex = tex_result.value();
				if (tex.map().size() != 1)
					return Unexpected("Framebuffer " + name + " target texture should have only one key:value pair: name and format");

				const auto &[tex_name, tex_type_name] = *tex.map().begin();
				if (fb.textures_count >= MAX_TARGET_TEXTURES)
					return Unexpected(format("Too many targets for framebuffer %s, max: %d",
						name.c_str(), MAX_TARGET_TEXTURES));

				const PixelType tex_type = pixelTypeFromString(tex_type_name.getString());

				const size_t tex_index = names_.texture.size();
				assert(tex_index == pipeline_.textures.size());

				if (pingpong) {
					addTexture(tex_index, Command::Index::Pingpong::Ping, name + "@ping." + tex_name, width, height, tex_type);
					addTexture(tex_index, Command::Index::Pingpong::Pong, name + "@pong." + tex_name, width, height, tex_type);
				} else {
					addTexture(tex_index, Command::Index::Pingpong::Fixed, name + "." + tex_name, width, height, tex_type);
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

		return Expected<void, std::string>();
	}

	Expected<void,std::string> loadPrograms() {
		auto map_programs_result = root_.getMapping("programs");
		if (!map_programs_result)
			return Unexpected(map_programs_result.error());
		for (const auto &[name, yprog]: map_programs_result.value().get().map()) {
			auto yp_result = yprog.getMapping();
			if (!yp_result)
				return Unexpected("Program " + name + " desc object is not a mapping");
			const yaml::Mapping &yp = yp_result.value();

			const int fragment_index = getShaderIndex(yp.getString("fragment"));
			const int vertex_index = getShaderIndex(yp.getString("vertex"));

			names_.program.emplace_back(name);
			pipeline_.programs.emplace_back(vertex_index, fragment_index);
		}

		return Expected<void,std::string>();
	}

	Expected<void,std::string> loadCommands() {
		auto paint_result = root_.getSequence("paint");
		if (!paint_result)
			return Unexpected(paint_result.error());

		for (const auto &ycmd_value: paint_result.value().get()) {
			if (ycmd_value.isString() && ycmd_value.getString() == "drawFullscreen") {
				pipeline_.commands.emplace_back(Command::DrawFullscreen());
				continue;
			}

			auto ycmd_result = ycmd_value.getMapping();
			if (!ycmd_result)
				return Unexpected<std::string>("Command is not a mapping");
			const yaml::Mapping &ycmd = ycmd_result.value();
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
				auto yunitex_result = yargs.getMapping();
				if (!yunitex_result)
					return Unexpected<std::string>("bindTexture argument is not a mapping");
				for (const auto &[yuniform, ytexture]: yunitex_result.value().get().map()) {
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
				auto ymap_result = yargs.getMapping();
				if (!ymap_result)
					return Unexpected<std::string>("drawArray argument is not a mapping");
				const yaml::Mapping &ymap = ymap_result.value();
				pipeline_.commands.emplace_back(
					Command::DrawArrays(
						drawModeFromString(ymap.getString("mode")),
						readVariable(ymap.getString("start")),
						readVariable(ymap.getString("count"))
					)
				);
			} else
				return Unexpected(format("Unknown command %s", op.c_str()));
		}

		return Expected<void,std::string>();
	}

public:
	Loader(Pipeline &pipeline, const yaml::Mapping &root)
		: pipeline_(pipeline)
		, root_(root)
	{
	}

	Expected<void, std::string> load() {
		if (root_.hasKey("framebuffers")) {
			auto result = loadFramebuffers();
			if (!result)
				return Unexpected(result.error());
		}

		{
			auto result = loadPrograms();
			if (!result)
				return Unexpected(result.error());
		}

		{
			auto result = loadCommands();
			if (!result)
				return Unexpected(result.error());
		}

		return Expected<void, std::string>();
	}
};

Expected<Pipeline, std::string> Pipeline::load(std::string_view s) {
	const auto root = yaml::parse(s);
	if (!root)
		return Unexpected("Cannot load pipeline: " + root.error());

	auto root_mapping_result = root.value().getMapping();
	if (!root_mapping_result)
		return Unexpected<std::string>("Root pipeline desc object is not a mapping");

	Pipeline pipeline;
	Loader loader(pipeline, root_mapping_result.value());
	const auto result = loader.load();
	if (!result)
		return Unexpected("Cannot load pipeline: " + result.error());

	return pipeline;
}

} // namespace renderdesc
