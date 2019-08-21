#pragma once

#include <string>
#include <vector>

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
};
