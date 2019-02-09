#include "Export.h"
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

static void writeUniformTrackData(FILE *out, const std::string &name) {
	std::string filename = "sync_" + name + ".track";

	const auto in = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename.c_str(), "rb"), &fclose);
	if (!in)
		throw std::runtime_error(format("Cannot open file '%s' for reading", filename.c_str()));

	fseek(in.get(), 0, SEEK_END);
	const size_t size = ftell(in.get());
	fseek(in.get(), 0, SEEK_SET);

	std::vector<unsigned char> data;
	data.resize(size);

	const size_t read = fread(data.data(), 1, size, in.get());
	if (size != read)
		throw std::runtime_error(format("Could read only %d of %d bytes from '%s'", (int)read, (int)size, filename.c_str()));

	fprintf(out, "\t{\"%s\", %d, \"", filename.c_str(), (int)size);
	for (const auto &c: data)
		fprintf(out, "\\x%x", c);
	fprintf(out, "\", 0},\n");
}

void exportC(const renderdesc::Pipeline &p, int w, int h, const char *filename) {
	MSG("Exporting rendering pipeline to '%s'", filename);

	const auto f = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename, "w"), &fclose);
	if (!f)
		throw std::runtime_error(format("Cannot open file '%s' for writing", filename));

	Resources res;
	{
		for (const auto &s: p.shader_filenames) {
			const std::string vname = validateName(s);
			const auto shader = res.getShaderSource(s);
			if (!shader)
				throw std::runtime_error(format("Cannot open shader '%s'", s.c_str()));

			if (!shader->poll(1))
				throw std::runtime_error(format("Cannot read shader '%s'", s.c_str()));

			fprintf(f.get(), "static const char %s[] =\n", vname.c_str());

			const std::string srn = "\n";
			const std::string src = shader->sources();
			for (size_t pos = 0; pos < src.length();) {
				size_t rn = src.find(srn, pos);
				if (rn == std::string::npos)
					rn = src.length();

				fprintf(f.get(), "\t\"%.*s\\n\"\n", (int)(rn - pos), src.c_str() + pos);
				pos = rn + srn.length();
			}

			fprintf(f.get(), ";\n\n");
		}
	}

	if (!p.textures.empty())
		fprintf(f.get(), "static GLuint textures[%d];\n", (int)p.textures.size());

	if (!p.framebuffers.empty())
		fprintf(f.get(), "static GLuint framebuffers[%d];\n", (int)p.textures.size());

	fprintf(f.get(), "static GLuint programs[%d];\n", (int)p.programs.size());

	std::vector<std::string> rocket_tracks;
	std::vector<std::pair<int,shader::UniformType>> rocket_tracks_details;
	std::vector<shader::UniformsMap> program_uniforms;
	int rocket_track_index = 0;
	for (const auto& prog: p.programs) {
		const auto vertex = res.getShaderSource(p.shader_filenames[prog.vertex]);
		const auto fragment = res.getShaderSource(p.shader_filenames[prog.fragment]);

		// Polled shaders are expected to be loaded
		auto uniforms = vertex->uniforms();
		const auto result = shader::appendUniforms(uniforms, fragment->uniforms());
		if (!result.hasValue())
			throw std::runtime_error("Error merging uniforms: " + result.error());
		for (const auto &[name, decl]: uniforms) {
			if (name != "R" && name != "t"
				&& std::find(rocket_tracks.begin(), rocket_tracks.end(), name) == rocket_tracks.end()) {

				rocket_tracks.push_back(name);
				rocket_tracks_details.emplace_back(rocket_track_index, decl.type);

				switch (decl.type) {
					case shader::UniformType::Float:
						rocket_track_index += 1;
						break;
					case shader::UniformType::Vec2:
						rocket_track_index += 2;
						break;
					case shader::UniformType::Vec3:
						rocket_track_index += 3;
						break;
					case shader::UniformType::Vec4:
						rocket_track_index += 4;
						break;
				}
			}
		}
		program_uniforms.push_back(std::move(uniforms));
	}

	fprintf(f.get(), "static struct RocketTrack rocket_tracks[%d] = {\n", rocket_track_index);
	for (size_t i = 0; i < rocket_tracks.size(); ++i) {
		const auto &name = rocket_tracks[i];
		const auto type = rocket_tracks_details[i].second;

		switch (type) {
			case shader::UniformType::Float:
				writeUniformTrackData(f.get(), name);
				break;
			case shader::UniformType::Vec2:
				writeUniformTrackData(f.get(), name + ".x");
				writeUniformTrackData(f.get(), name + ".y");
				break;
			case shader::UniformType::Vec3:
				writeUniformTrackData(f.get(), name + ".x");
				writeUniformTrackData(f.get(), name + ".y");
				writeUniformTrackData(f.get(), name + ".z");
				break;
			case shader::UniformType::Vec4:
				writeUniformTrackData(f.get(), name + ".x");
				writeUniformTrackData(f.get(), name + ".y");
				writeUniformTrackData(f.get(), name + ".z");
				writeUniformTrackData(f.get(), name + ".w");
				break;
		}
	}
	fprintf(f.get(), "};\n\n");

	if (rocket_track_index)
		fprintf(f.get(), "static const struct sync_track *tracks[%d];\n", rocket_track_index);

	fprintf(f.get(), "\nstatic void videoInit() {\n");

	rocket_track_index = 0;
	for (size_t i = 0; i < rocket_tracks.size(); ++i) {
		const auto &name = rocket_tracks[i];
		const auto type = rocket_tracks_details[i].second;

		switch (type) {
			case shader::UniformType::Float:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, name.c_str());
				break;
			case shader::UniformType::Vec2:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				break;
			case shader::UniformType::Vec3:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".z").c_str());
				break;
			case shader::UniformType::Vec4:
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".x").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".y").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".z").c_str());
				fprintf(f.get(), "\ttracks[%d] = sync_get_track(rocket_device, \"%s\");\n", rocket_track_index++, (name + ".w").c_str());
				break;
			}
	}

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
				case RGBA16F:
					comp = "GL_RGBA16F";
					type = "GL_FLOAT";
					break;
				case RGBA32F:
					comp = "GL_RGBA32F";
					type = "GL_FLOAT";
					break;
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
						fprintf(f.get(), "\tglViewport(0, 0, %d, %d);\n", w, h);
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
					fprintf(f.get(), "\tglUniform2f(glGetUniformLocation(current_program, \"R\"), %d, %d);\n", w, h);
					fprintf(f.get(), "\tglUniform1f(glGetUniformLocation(current_program, \"t\"), t);\n");
					const auto &uniforms = program_uniforms[pi];
					for (const auto &[name, decl]: uniforms) {
						const int track_index = (int)(std::find(rocket_tracks.begin(), rocket_tracks.end(), name) - rocket_tracks.begin());
						if (track_index != (int)rocket_tracks.size()) {
							const auto &index_type = rocket_tracks_details[track_index];
							switch (decl.type) {
								case shader::UniformType::Float:
									fprintf(f.get(), "\tglUniform1f(glGetUniformLocation(current_program, \"%s\"), sync_get_val(tracks[%d], t));\n", name.c_str(), index_type.first);
									break;
								case shader::UniformType::Vec2:
									fprintf(f.get(), "\tglUniform2f(glGetUniformLocation(current_program, \"%s\"), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t));\n", name.c_str(),
											index_type.first,
											index_type.first+1);
									break;
								case shader::UniformType::Vec3:
									fprintf(f.get(), "\tglUniform3f(glGetUniformLocation(current_program, \"%s\"), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t));\n", name.c_str(),
											index_type.first,
											index_type.first+1,
											index_type.first+2);
									break;
								case shader::UniformType::Vec4:
									fprintf(f.get(), "\tglUniform4f(glGetUniformLocation(current_program, \"%s\"), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t), "
											"sync_get_val(tracks[%d], t));\n", name.c_str(),
											index_type.first,
											index_type.first+1,
											index_type.first+2,
											index_type.first+3);
									break;
								}
						}
					}
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
					case renderdesc::Command::Flag::VertexProgramPointSize:
						fprintf(f.get(), "\tglEnable(GL_VERTEX_PROGRAM_POINT_SIZE);\n");
						break;
				}
				break;
			case renderdesc::Command::Op::Disable:
				switch (cmd.enable.flag) {
					case renderdesc::Command::Flag::DepthTest:
						fprintf(f.get(), "\tglDisable(GL_DEPTH_TEST);\n");
						break;
					case renderdesc::Command::Flag::VertexProgramPointSize:
						fprintf(f.get(), "\tglDisable(GL_VERTEX_PROGRAM_POINT_SIZE);\n");
						break;
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
}
