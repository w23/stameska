#include "GuiScope.h"
#include "imgui.h"

GuiScope::GuiScope() {}

Value GuiScope::getValue(const std::string& name, int comps) {
	auto it = vars_.find(name);
	if (it != vars_.end()) {
		it->second.components = comps;
		return it->second.value;
	}

	return (vars_[name] = Variable{comps, Value{0}}).value;
}

void GuiScope::update(float row) {
	(void)row;
}

void GuiScope::save() const {
	// Not implemented
}

Expected<IAutomation::ExportResult, std::string> GuiScope::writeExport(std::string_view config, const shader::UniformsMap &uniforms) const {
	return Unexpected(std::string("Not implemented"));
}

void GuiScope::paint() {
	if (ImGui::Begin("Uniforms")) {
		for(auto &[name, v]: vars_) {
			//MSG("%s -> %d", name.c_str(), v.components);
			//ImGui::LabelText("", "%s", name.c_str());
			ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, &v.value.x, v.components, .01f);
		}
	}

	ImGui::End();
}
