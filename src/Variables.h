#pragma once

#include "Expected.h"
#include "ShaderSource.h"

#include <string>
#include <vector>

struct Section {
	enum class Type {
		Data,
		Code,
	} type;

	std::string name;
	std::string comment;
	std::vector<char> data;
};

struct Value {
	float x, y, z, w;
};

class IScope {
public:
	virtual ~IScope() {}

	virtual Value getValue(const std::string& name, int comps) = 0;
};

class DummyScope : public IScope {
public:
	virtual Value getValue(const std::string&, int) override { return Value{0,0,0,0}; }
};

class IAutomation : public IScope {
public:
	virtual ~IAutomation() {}

	virtual void update(float row) = 0;
	virtual void save() const = 0;

	struct ExportedUniform {
		int offset; // < 0 == constant
		Value constant;

		ExportedUniform(Value constant) : offset(-1), constant(constant) {}
		ExportedUniform(int offset) : offset(offset) {}
	};

	struct ExportResult {
		std::vector<Section> sections;
		std::map<std::string, ExportedUniform> uniforms;
		int buffer_size;
	};

	virtual Expected<ExportResult, std::string> writeExport(std::string_view config, const shader::UniformsMap &uniforms) const = 0;
};
