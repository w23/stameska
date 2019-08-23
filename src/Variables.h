#pragma once

#include "Expected.h"
#include "ShaderSource.h"

#include <string>
#include <vector>

struct Section {
	enum class Type {
		Data,
		Code,
		BSS,
	} type;

	std::string name;
	std::string comment;
	size_t size; // For BSS, as data will be empty
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

	struct ExportConfig {
		std::string config;
		const shader::UniformsMap &uniforms;
	};

	using ConstantsMap = std::map<std::string_view, Value>;

	struct ExportResult {
		std::vector<Section> sections;
		ConstantsMap constants;
	};

	virtual Expected<ExportResult, std::string> writeExport(std::string_view config, const shader::UniformsMap &uniforms) const = 0;
};
