#pragma once

#include "Variables.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

class PolledFile;

class AutomationBasic : public IAutomation {
public:
	AutomationBasic(const std::string &filename);
	~AutomationBasic();

	virtual Value getValue(const std::string &name, int components) override;

	virtual void update(float row) override;
	virtual void save() const override {}

	virtual Expected<std::vector<Section>, std::string> writeExport(const ExportConfig &config) const override;

private:
	void reread();

	unsigned int seq_ = 0;
	std::unique_ptr<PolledFile> source_;

	struct Keypoint {
		float row;
		Value value;

		bool operator<(const Keypoint &lhs) { return row < lhs.row; }
	};

	using Sequence = std::vector<Keypoint>;
	using Data = std::map<std::string, Sequence>;

	Data data_;

	using Slice = std::map<std::string, Value>;
	Slice slice_;
};
