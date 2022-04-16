#include "Export.h"
#include "Variables.h"
#include "RenderDesc.h"
#include "Resources.h"
#include "PolledShaderSource.h"
#include "format.h"

#include <algorithm>
#include <memory>

static std::string validateName(const std::string &str) {
	std::string out;
	for (const auto &c: str) {
		if (isalnum(c) || c == '_') {
			out += c;
		} else if(c == '.') {
			out += '_';
		} else {
			char buf[5] = "_xx_";
			buf[1] = ((c>>4)&0xf)["0123456789ABCDEF"];
			buf[2] = (c&0xf)["0123456789ABCDEF"];
			out += buf;
		}
	}

	return out;
}

static Expected<void, std::string> writeFile(const std::string &filename, const void *data, size_t bytes) {
	const auto out = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename.c_str(), "wb"), &fclose);
	if (!out)
		return Unexpected(format("Cannot open file '%s' for writing", filename.c_str()));

	const size_t written = fwrite(data, 1, bytes, out.get());
	if (written != bytes)
		return Unexpected(format("Cannot write %zu bytes, could write only %zu", bytes, written));

	return Expected<void, std::string>();
}

static Expected<void, std::string> writeShaderSource(FILE *main, const std::string &origin_filename, const std::string &name, const std::string &src, const ExportSettings &settings) {
	if (!settings.shader_path.empty()) {
		const std::string filename = settings.shader_path + origin_filename;
		MSG("shader %s -> %s", name.c_str(), filename.c_str());
		return writeFile(filename, src.data(), src.size());
	} else {
		fprintf(main, "static const char %s[] =\n", name.c_str());

		const std::string srn = "\n";
		for (size_t pos = 0; pos < src.length();) {
			size_t rn = src.find(srn, pos);
			if (rn == std::string::npos)
				rn = src.length();

			fprintf(main, "\t\"%.*s\\n\"\n", static_cast<int>(rn - pos), src.c_str() + pos);
			pos = rn + srn.length();
		}

		fprintf(main, ";\n\n");
	}

	return Expected<void, std::string>();
}

static Expected<std::string, std::string> shaderPreprocessor(const shader::Source &flat, const IAutomation::ExportResult &export_result) {
	std::string source;
	if (flat.version() != 0)
		source = format("#version %d\n", flat.version());
	if (export_result.buffer_size > 0)
		source += format("uniform float u[%d];\n", export_result.buffer_size);

	for (const auto &chunk: flat.chunks()) {
		switch (chunk.type) {
			case shader::Source::Chunk::Type::Uniform:
				{
					const auto uit = flat.uniforms().find(chunk.value);
					if (uit == flat.uniforms().end())
						return Unexpected(format("Cannot find uniform chunk '%s' in uniforms", chunk.value.c_str()));

					const auto eit = export_result.uniforms.find(chunk.value);
					if (eit == export_result.uniforms.end())
						return Unexpected(format("Cannot find uniform '%s' in exported uniforms", chunk.value.c_str()));

					if (eit->second.offset < 0) {
						const Value value = eit->second.constant;
						switch(uit->second.type) {
							case shader::UniformType::Float:
								source += format("%f", value.x);
								break;
							case shader::UniformType::Vec2:
								source += format("vec2(%f,%f)", value.x, value.y);
								break;
							case shader::UniformType::Vec3:
								source += format("vec3(%f,%f,%f)", value.x, value.y, value.z);
								break;
							case shader::UniformType::Vec4:
								source += format("vec4(%f,%f,%f,%f)", value.x, value.y, value.z, value.w);
								break;
						}
					} else {
						const int offset = eit->second.offset;
						switch(uit->second.type) {
							case shader::UniformType::Float:
								source += format("u[%d]", offset);
								break;
							case shader::UniformType::Vec2:
								source += format("vec2(u[%d],u[%d])", offset, offset+1);
								break;
							case shader::UniformType::Vec3:
								source += format("vec3(u[%d],u[%d],u[%d])", offset, offset+1, offset+2);
								break;
							case shader::UniformType::Vec4:
								source += format("vec4(u[%d],u[%d],u[%d],u[%d])", offset, offset+1, offset+2, offset+3);
								break;
						}
					}
				}
				break;
			case shader::Source::Chunk::Type::String:
				source += chunk.value;
				break;
			case shader::Source::Chunk::Type::Include:
				return Unexpected<std::string>("Include chunk is invalid in flat shader source");
		}
	}

	return Expected<std::string, std::string>(std::move(source));
}

Expected<void, std::string> exportC(Resources &res, const ExportSettings &settings, const renderdesc::Pipeline &p, const IAutomation &automation) {
	const char * const filename = settings.c_source.c_str();
	MSG("Exporting rendering pipeline to '%s'", filename);

	const auto f = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename, "w"), &fclose);
	if (!f)
		return Unexpected(format("Cannot open file '%s' for writing", filename));

	// Extract all uniforms from all shaders
	shader::UniformsMap global_uniforms;
	for (const auto &s: p.shader_filenames) {
		const auto shader = res.getShaderSource(s);
		if (!shader)
			return Unexpected(format("Cannot open shader '%s'", s.c_str()));

		// if (!shader->poll(1))
		// 	return Unexpected(format("Cannot read shader '%s'", s.c_str()));

		MSG("%s: uniforms: %d", s.c_str(), (int)shader->flatSource().uniforms().size());
		const auto result = shader::appendUniforms(global_uniforms, shader->flatSource().uniforms());
		if (!result)
			return Unexpected("Error merging uniforms from vertex: " + result.error());
	}

	// Export automation for all known uniforms
	auto automation_export = automation.writeExport("C", global_uniforms);
	if (!automation_export)
		return Unexpected(format("Error exporting automation data: %s", automation_export.error().c_str()));

	const IAutomation::ExportResult automation_result(std::move(automation_export.value()));

	// write shader sources given automation export
	for (const auto &s: p.shader_filenames) {
		const std::string vname = validateName(s);
		const auto shader = res.getShaderSource(s);

		auto shader_source = shaderPreprocessor(shader->flatSource(), automation_result);
		if (!shader_source)
			return Unexpected(format("Cannot preporcess shader '%s': %s", s.c_str(), shader_source.error().c_str()));

		if (auto result = writeShaderSource(f.get(), s, vname, shader_source.value(), settings)) {}
		else
			return Unexpected(format("Cannot write shader '%s': %s", vname.c_str(), result.error().c_str()));
	}

	// Write global data
	for (const auto &it: automation_result.sections) {
		if (it.type == Section::Type::Data) {
			fprintf(f.get(), "#pragma data_seg(\".%s\")\n", it.name.c_str());
			fprintf(f.get(), "%.*s\n", static_cast<int>(it.data.size()), it.data.data());
		}
	}

	if (!p.textures.empty())
		fprintf(f.get(), "static GLuint textures[%d];\n", (int)p.textures.size());

	if (!p.framebuffers.empty())
		fprintf(f.get(), "static GLuint framebuffers[%d];\n", (int)p.textures.size());

	fprintf(f.get(), "static GLuint programs[%d];\n", (int)p.programs.size());

	fprintf(f.get(), "\nstatic void videoInit() {\n");

	if (!p.textures.empty())
		fprintf(f.get(), "\tglGenTextures(%d, textures);\n", (int)p.textures.size());

	if (!p.framebuffers.empty())
		fprintf(f.get(), "\tglGenFramebuffers(%d, framebuffers);\n", (int)p.framebuffers.size());

	if (!p.textures.empty()) {
		for (size_t i = 0; i < p.textures.size(); ++i) {
			const renderdesc::Texture &t = p.textures[i];
			const char *comp = "", *type = "";

			switch (t.pixel_type) {
				case RGBA8:
					comp = "GL_RGBA";
					type = "GL_UNSIGNED_BYTE";
					break;
#ifndef ATTO_PLATFORM_RPI
				case RGBA16F:
					comp = "GL_RGBA16F";
					type = "GL_FLOAT";
					break;
				case RGBA32F:
					comp = "GL_RGBA32F";
					type = "GL_FLOAT";
					break;
#endif
			}

			fprintf(f.get(), "\ttextureInit(textures[%d], %d, %d, %s, %s);\n",
				(int)i, t.w, t.h, comp, type);
		}

		fprintf(f.get(), "\n");
	}

	if (!p.framebuffers.empty()) {
		for (size_t i = 0; i < p.framebuffers.size(); ++i) {
			const renderdesc::Framebuffer &fb = p.framebuffers[i];
			fprintf(f.get(), "\tglBindFramebuffer(GL_FRAMEBUFFER, framebuffers[%d]);\n", (int)i);
			for (int ti = 0; ti < fb.textures_count; ++ti) {
				fprintf(f.get(), "\tglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + %d, "
					"GL_TEXTURE_2D, textures[%d], 0);\n", ti, fb.textures[ti]);
			}
		}

		fprintf(f.get(), "\n");
	}

	for (size_t i = 0; i < p.programs.size(); ++i) {
		fprintf(f.get(), "\tprograms[%d] = programInit(%s, %s);\n", (int)i,
			validateName(p.shader_filenames[p.programs[i].vertex]).c_str(),
			validateName(p.shader_filenames[p.programs[i].fragment]).c_str());
	}

	fprintf(f.get(), "}\n\n");

	fprintf(f.get(), "static void videoPaint(float t) {\n");

	const char *pingpong[3] = {"", "+ping", "+pong"};
	fprintf(f.get(),
		"\tstatic unsigned int frame_seq = 0;\n"
		"\t++frame_seq;\n"
		"\tconst int ping = (int)(frame_seq&1), pong = (int)((frame_seq+1)&1);\n");

	int first_texture_slot = 0;
	for (const auto &cmd: p.commands) {
		switch (cmd.op) {
			case renderdesc::Command::Op::BindFramebuffer:
				{
					const renderdesc::Command::BindFramebuffer &cmdfb = cmd.bindFramebuffer;
					if (cmdfb.framebuffer.index == -1) {
						fprintf(f.get(), "\tglBindFramebuffer(GL_FRAMEBUFFER, 0);\n");
						fprintf(f.get(), "\tglViewport(0, 0, %d, %d);\n", settings.width, settings.height);
					} else {
						const renderdesc::Framebuffer &fb = p.framebuffers[cmdfb.framebuffer.index];
						fprintf(f.get(), "\tglBindFramebuffer(GL_FRAMEBUFFER, framebuffers[%d%s]);\n",
							cmdfb.framebuffer.index, pingpong[cmdfb.framebuffer.pingpong]);
						fprintf(f.get(), "\tglDrawBuffers(%d, draw_buffers);\n", fb.textures_count);
						fprintf(f.get(), "\tglViewport(0, 0, %d, %d);\n",
							p.textures[fb.textures[0]].w,
							p.textures[fb.textures[0]].h);
					}
					break;
				}

			case renderdesc::Command::Op::UseProgram:
				{
					const renderdesc::Command::UseProgram &cmdp = cmd.useProgram;
					const int pi = cmdp.program.index;
					fprintf(f.get(), "\tcurrent_program = programs[%d];\n", pi);
					fprintf(f.get(), "\tglUseProgram(current_program);\n");
					fprintf(f.get(), "\tglUniform2f(glGetUniformLocation(current_program, \"R\"), %d, %d);\n", settings.width, settings.height);
					fprintf(f.get(), "\tglUniform1f(glGetUniformLocation(current_program, \"t\"), t);\n");

					// FIXME load automated uniforms

					first_texture_slot = 0;
					break;
				}

			case renderdesc::Command::Op::BindTexture:
				{
					const auto &cmdtex = cmd.bindTexture;
					const int slot = first_texture_slot++;
					fprintf(f.get(), "\tglActiveTexture(GL_TEXTURE0 + %d);\n", slot);
					fprintf(f.get(), "\tglBindTexture(GL_TEXTURE_2D, textures[%d%s]);\n", cmdtex.texture.index, pingpong[cmdtex.texture.pingpong]);
					fprintf(f.get(), "\tglUniform1i(glGetUniformLocation(current_program, \"%s\"), %d);\n", cmdtex.name.c_str(), slot);
					break;
				}

			case renderdesc::Command::Op::Clear:
				{
					const auto &cmdc = cmd.clear;
					fprintf(f.get(), "\tglClearColor(%f, %f, %f, %f);\n", cmdc.r, cmdc.g, cmdc.b, cmdc.a);
					fprintf(f.get(), "\tglClear(GL_COLOR_BUFFER_BIT%s);\n", cmdc.depth?" | GL_DEPTH_BUFFER_BIT":"");
					break;
				}

			case renderdesc::Command::Op::Enable:
				switch (cmd.enable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						fprintf(f.get(), "\tglEnable(GL_DEPTH_TEST);\n");
						break;
#ifndef ATTO_PLATFORM_RPI
					case renderdesc::Command::Flag::VertexProgramPointSize:
						fprintf(f.get(), "\tglEnable(GL_VERTEX_PROGRAM_POINT_SIZE);\n");
						break;
#endif
				}
				break;
			case renderdesc::Command::Op::Disable:
				switch (cmd.enable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						fprintf(f.get(), "\tglDisable(GL_DEPTH_TEST);\n");
						break;
#ifndef ATTO_PLATFORM_RPI
					case renderdesc::Command::Flag::VertexProgramPointSize:
						fprintf(f.get(), "\tglDisable(GL_VERTEX_PROGRAM_POINT_SIZE);\n");
						break;
#endif
				}
				break;
			case renderdesc::Command::Op::DrawArrays:
				{
					const char *mode = nullptr;
					switch (cmd.drawArrays.mode) {
						case renderdesc::Command::DrawArrays::Mode::Lines: mode = "GL_LINES"; break;
						case renderdesc::Command::DrawArrays::Mode::LineStrip: mode = "GL_LINE_STRIP"; break;
						case renderdesc::Command::DrawArrays::Mode::LineLoop: mode = "GL_LINE_LOOP"; break;
						case renderdesc::Command::DrawArrays::Mode::Points: mode = "GL_POINTS"; break;
						case renderdesc::Command::DrawArrays::Mode::Triangles: mode = "GL_TRIANGLES"; break;
						case renderdesc::Command::DrawArrays::Mode::TriangleStrip: mode = "GL_TRIANGLE_STRIP"; break;
						case renderdesc::Command::DrawArrays::Mode::TriangleFan: mode = "GL_TRIANGLE_FAN"; break;
					}
					fprintf(f.get(), "\tglDrawArrays(%s, %d, %d);\n", mode, cmd.drawArrays.start, cmd.drawArrays.count);
				}
				break;
			case renderdesc::Command::Op::DrawFullscreen:
				fprintf(f.get(), "\tglRects(-1,-1,1,1);\n");
				break;
		}
	}

	fprintf(f.get(), "}\n");
	MSG("Done exporting rendering pipeline to '%s'", filename);

	return Expected<void, std::string>();
}
