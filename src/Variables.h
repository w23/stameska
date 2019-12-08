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
	//virtual Value getValue(const std::string& name, int comps) const = 0;
};

/*
class NonCopyable {
protected:
	NonCopyable() {}

private:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

class ScopeMultiplexer : public IScope, NonCopyable {
public:
	ScopeMultiplexer(std::tuple<const IScope&> scopes) : scopes_(std::move(scopes)) {}

	virtual Value getValue(const std::string& name, int comps) override;

private:
	const std::tuple<const IScope&> scopes_;
};

class ScopeMultiplexerDynamic : public IScope, NonCopyable {
public:
	void push_front(const IScope *scope) { scopes_.push_back(scope); }
	void push_back(const IScope *scope) { scopes_.insert(scopes_.begin(), scope); }

	virtual Value getValue(const std::string& name, int comps) override {
	}

private:
	std::vector<const IScope*> scopes_;
};
*/

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
