#include "RenderDesc.h"
#include "YamlParser.h"
#include "format.h"

#include <assert.h>
#include <algorithm>

namespace renderdesc {

static Expected<PixelType, std::string> pixelTypeFromString(const std::string& s) {
	if (s == "RGBA8") return PixelType::RGBA8;
#ifndef ATTO_PLATFORM_RPI
	if (s == "RGBA16F") return PixelType::RGBA16F;
	if (s == "RGBA32F") return PixelType::RGBA32F;
#endif
	//if (s == "Depth24") return PixelType::Depth24;

	return Unexpected(format("Unexpected pixel type %s", s.c_str()));
}

static Expected<PixelType, std::string> pixelTypeFromValue(const yaml::Value &v) {
	auto s_result = v.getString();
	if (!s_result)
		return Unexpected("Cannot read PixelType: " + s_result.error());

	return pixelTypeFromString(s_result.value());
}

static Expected<Command::Flag, std::string> flagFromValue(const yaml::Value &v) {
	auto s_result = v.getString();
	if (!s_result)
		return Unexpected("Cannot read Flag: " + s_result.error());

	const std::string &s = s_result.value();
	if (s == "DepthTest") return Command::Flag::DepthTest;
#ifndef ATTO_PLATFORM_RPI
	if (s == "VertexProgramPointSize") return Command::Flag::VertexProgramPointSize;
#endif

	return Unexpected(format("Unknown flag %s", s.c_str()));
}

static Expected<Command::DrawArrays::Mode, std::string> drawModeFromString(const std::string &s) {
	if (s == "Points") return Command::DrawArrays::Mode::Points;
	if (s == "Triangles") return Command::DrawArrays::Mode::Triangles;
	if (s == "TriangleStrip") return Command::DrawArrays::Mode::TriangleStrip;
	if (s == "TriangleFan") return Command::DrawArrays::Mode::TriangleFan;
	if (s == "Lines") return Command::DrawArrays::Mode::Lines;
	if (s == "LineStrip") return Command::DrawArrays::Mode::LineStrip;
	if (s == "LineLoop") return Command::DrawArrays::Mode::LineLoop;

	return Unexpected(format("Unknown draw mode %s", s.c_str()));
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

	Expected<long int, std::string> readVariable(const yaml::Value &v) {
		// FIXME read actual variable
		return v.getInt();
	}

	Expected<long int, std::string> readVariable(const yaml::Mapping &m, const std::string &name) {
		auto value_result = m.getValue(name);
		if (!value_result)
			return Unexpected("Cannot read variable " + name + ": " + value_result.error());
		// FIXME read actual variable
		return value_result.value().get().getInt();
	}

	Expected<Command::Index, std::string> getFramebufferIndex(const std::string &s) {
		if (s == "SCREEN")
			return Command::Index(-1);

		const auto it = std::find(names_.framebuffer.begin(), names_.framebuffer.end(), s);
		if (it == names_.framebuffer.end())
			return Unexpected(format("Unknown framebuffer %s", s.c_str()));

		return indexes_.framebuffer[it - names_.framebuffer.begin()];
	}

	Expected<Command::Index, std::string> getProgramIndex(const std::string &s) {
		const auto it = std::find(names_.program.begin(), names_.program.end(), s);
		if (it == names_.program.end())
			return Unexpected(format("Unknown program %s", s.c_str()));

		return Command::Index(it - names_.program.begin());
	}

	Expected<Command::Index, std::string> getTextureIndex(const std::string &s) {
		const auto it = std::find(names_.texture.begin(), names_.texture.end(), s);
		if (it == names_.texture.end())
			return Unexpected(format("Unknown texture %s", s.c_str()));

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

	Expected<void, std::string> registerTexture(int index, Command::Index::Pingpong pp, const std::string &name, Texture tex) {
		if (index < 0) {
			index = names_.texture.size();
			assert(index == pipeline_.textures.size());
		}

		for (const auto &it: names_.texture) {
			if (it == name)
				return Unexpected("Already have texture with name " + name);
		}

		names_.texture.emplace_back(name);
		pipeline_.textures.emplace_back(tex);
		indexes_.texture.emplace_back(Command::Index(index, pp));

		return Expected<void, std::string>();
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

			if (size.size() != 2)
				return Unexpected("Framebuffer " + name + " size must have 2 elements");

			auto width_result = readVariable(size[0]);
			if (!width_result)
				return Unexpected("Cannot read width for framebuffer " + name + ": " + width_result.error());
			const int width = width_result.value();

			auto height_result = readVariable(size[1]);
			if (!height_result)
				return Unexpected("Cannot read height for framebuffer " + name + ": " + height_result.error());
			const int height = height_result.value();

			const auto pingpong_result = yfb.getInt("pingpong");
			const bool pingpong = pingpong_result && pingpong_result.value();

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

				auto tex_type_result = pixelTypeFromValue(tex_type_name);
				if (!tex_type_result)
					return Unexpected(format("Framebuffer %s texture %d has invalid pixel type: %s",
						name.c_str(), fb.textures_count, tex_type_result.error().c_str()));
				const PixelType tex_type = tex_type_result.value();

				const size_t tex_index = names_.texture.size();
				assert(tex_index == pipeline_.textures.size());

				Texture texture{width, height, tex_type};
				if (pingpong) {
					{
						auto result = registerTexture(tex_index, Command::Index::Pingpong::Ping, name + "@ping." + tex_name, texture);
						if (!result)
							return Unexpected(result.error());
					}

					{
						auto result = registerTexture(tex_index, Command::Index::Pingpong::Pong, name + "@pong." + tex_name, texture);
						if (!result)
							return Unexpected(result.error());
					}
				} else {
					auto result = registerTexture(tex_index, Command::Index::Pingpong::Fixed, name + "." + tex_name, texture);
					if (!result)
						return Unexpected(result.error());
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
					indexes_.framebuffer.emplace_back(Command::Index(index));
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

	Expected<void,std::string> loadTextures() {
		auto map_textures_result = root_.getMapping("textures");
		if (!map_textures_result)
			return Unexpected(map_textures_result.error());

		for (const auto &[name, ytex]: map_textures_result.value().get().map()) {
			auto yt_result = ytex.getMapping();
			if (!yt_result)
				return Unexpected("Texture " + name + " desc object is not a mapping");
			const yaml::Mapping &yt = yt_result.value();

#define GET_STRING(collection, varname, name) \
			auto varname ## _result = collection.getString(name); \
			if (!varname ## _result) \
				return Unexpected(varname ## _result.error()); \
			const auto varname = varname ## _result.value()

			GET_STRING(yt, file, "file");
			return registerTexture(-1, Command::Index::Pingpong::Fixed, name, Texture{file});
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

			auto vertex_name_result = yp.getString("vertex");
			if (!vertex_name_result)
				return Unexpected("Program " + name + " invalid vertex: " + vertex_name_result.error());
			const int vertex_index = getShaderIndex(vertex_name_result.value());

			auto fragment_name_result = yp.getString("fragment");
			if (!fragment_name_result)
				return Unexpected("Program " + name + " invalid fragment: " + fragment_name_result.error());
			const int fragment_index = getShaderIndex(fragment_name_result.value());

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
			if (ycmd_value.isString() && ycmd_value.getString().value().get() == "drawFullscreen") {
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
				auto fb_name_result = yargs.getString();
				if (!fb_name_result)
					return Unexpected("Cannot read framebuffer name: " + fb_name_result.error());
				auto index_result = getFramebufferIndex(fb_name_result.value());

				if (!index_result)
					return Unexpected(index_result.error());

				pipeline_.commands.emplace_back(Command::BindFramebuffer(index_result.value()));
			} else if (op == "useProgram") {
				auto prog_name_result = yargs.getString();
				if (!prog_name_result)
					return Unexpected("Cannot read program name: " + prog_name_result.error());
				auto index_result = getProgramIndex(prog_name_result.value());
				if (!index_result)
					return Unexpected(index_result.error());
				pipeline_.commands.emplace_back(Command::UseProgram(index_result.value()));
			} else if (op == "bindTexture") {
				auto yunitex_result = yargs.getMapping();
				if (!yunitex_result)
					return Unexpected<std::string>("bindTexture argument is not a mapping");
				for (const auto &[yuniform, ytexture]: yunitex_result.value().get().map()) {
					auto tex_name_result = ytexture.getString();
					if (!tex_name_result)
						return Unexpected("Cannot read texture name for uniform " + yuniform + ": " + tex_name_result.error());
					auto index_result = getTextureIndex(tex_name_result.value());
					if (!index_result)
						return Unexpected(index_result.error());
					pipeline_.commands.emplace_back(
						Command::BindTexture(yuniform, index_result.value()));
				}
			} else if (op == "clear") {
				auto colors = yargs.getSequence();
				if (!colors)
					return Unexpected("Cannot read clear colors: " + colors.error());
				const auto& color_seq = colors.value().get();
				if (color_seq.size() != 4)
					return Unexpected(format("Clear colors should have 4 components instead of %d", color_seq.size()));
				
				float color[4];
				for (int i = 0; i < 4; ++i) {
					const auto value = color_seq[i].getFloat();
					if (!value)
						return Unexpected(format("Cannot read clear color component %d: %s", i, value.error().c_str()));
					color[i] = value.value();
				}

				pipeline_.commands.emplace_back(
					Command::Clear(color[0], color[1], color[2], color[3], true));
			} else if (op == "enable") {
				auto flag_result = flagFromValue(yargs);
				if (!flag_result)
					return Unexpected(flag_result.error());
				pipeline_.commands.emplace_back(Command::Enable(flag_result.value()));
			} else if (op == "disable") {
				auto flag_result = flagFromValue(yargs);
				if (!flag_result)
					return Unexpected(flag_result.error());
				pipeline_.commands.emplace_back(Command::Disable(flag_result.value()));
			} else if (op == "drawArrays") {
				auto ymap_result = yargs.getMapping();
				if (!ymap_result)
					return Unexpected<std::string>("drawArray argument is not a mapping");
				const yaml::Mapping &ymap = ymap_result.value();

				auto mode_result_str = ymap.getString("mode");
				if (!mode_result_str)
					return Unexpected("Cannot read draw mode string: " + mode_result_str.error());

				auto mode_result = drawModeFromString(mode_result_str.value());
				if (!mode_result)
					return Unexpected("Cannot read draw mode: " + mode_result.error());

				auto start_result = readVariable(ymap, "start");
				if (!start_result)
					return Unexpected("Cannot read draw start: " + start_result.error());

				auto count_result = readVariable(ymap, "count");
				if (!count_result)
					return Unexpected("Cannot read draw count: " + count_result.error());

				pipeline_.commands.emplace_back(
					Command::DrawArrays(
						mode_result.value(),
						(int)start_result.value(),
						(int)count_result.value()
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

		if (root_.hasKey("textures")) {
			auto result = loadTextures();
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
