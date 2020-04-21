#include "Variables.h"
#include <unordered_map>

class GuiScope : public IAutomation {
public:
	GuiScope();

	virtual Value getValue(const std::string& name, int comps) override;
	virtual void update(float row) override;
	virtual void save() const override;
	virtual Expected<ExportResult, std::string> writeExport(std::string_view config, const shader::UniformsMap &uniforms) const override;

	void paint() override;

private:
	struct Variable {
		int components;
		Value value;
	};

	std::unordered_map<std::string, Variable> vars_;
};
