#include "AutomationBasic.h"
#include "PolledFile.h"
#include "YamlParser.h"

AutomationBasic::AutomationBasic(const std::string &filename)
	: source_(new PolledFile(filename))
{
}

AutomationBasic::~AutomationBasic() {
}

Value AutomationBasic::getValue(const std::string &name, int) {
	const auto it = slice_.find(name);
	if (slice_.end() == it)
		return Value{0,0,0,0};

	return it->second;
}

void AutomationBasic::reread() {
	if (!source_->poll(++seq_))
		return;

	auto parsed = yaml::parse(source_->string());
	if (!parsed) {
		MSG("Automation parsing failed: %s", parsed.error().c_str());
		return;
	}
	auto map = parsed->getMapping();
	if (!map) {
		MSG("Automation must start with mapping: %s", map.error().c_str());
		return;
	}

	Data new_data;

	const yaml::Mapping &ymap = map.value();
	for(const auto &kv: ymap.map()) {
		const std::string &var = kv.first;
		auto sequence = kv.second.getSequence();
		if (!sequence) {
			MSG("Error parsing %s: %s", var.c_str(), sequence.error().c_str());
			return;
		}

		Sequence seq;

		const yaml::Sequence &yseq = sequence.value();
		for (const auto &it: yseq) {
			auto row = it.getSequence();
			if (!row) {
				MSG("Error parsing %s row: %s", var.c_str(), row.error().c_str());
				return;
			}

			const yaml::Sequence &yrow = row.value();
			if (yrow.size() < 2) {
				MSG("Error parsing %s row: should be at least [t, x]", var.c_str());
				return;
			}

#define READ_FLOAT(i, name, to) \
			do { \
				auto float_string = yrow[i].getString(); \
				if (!float_string) { \
					MSG("Error parsing %s row " #name ": %s", var.c_str(), float_string.error().c_str()); \
					return; \
				} \
				auto parsed_float = floatFromString(float_string.value()); \
				if (!parsed_float) { \
					MSG("Error parsing %s row " #name ": %s", var.c_str(), parsed_float.error().c_str()); \
					return; \
				} \
				to = parsed_float.value(); \
			} while(0)

			Keypoint kp = {0,{0,0,0,0}};
			READ_FLOAT(0, time, kp.row);
			READ_FLOAT(1, x, kp.value.x);
			if (yrow.size() > 2) READ_FLOAT(2, y, kp.value.y);
			if (yrow.size() > 3) READ_FLOAT(3, z, kp.value.z);
			if (yrow.size() > 4) READ_FLOAT(4, w, kp.value.w);

			if (!seq.empty() && seq.back().row > kp.row) {
				MSG("Error parsing %s rows: monotonic time expected", var.c_str());
				return;
			}

			seq.push_back(kp);
		}

		if (!seq.empty())
			new_data.emplace(std::move(var), std::move(seq));
	}

	data_ = std::move(new_data);
}

void AutomationBasic::update(float row) {
	reread();

	Slice slice;
	for (const auto &kv: data_) {
		size_t i;
		const Sequence &seq = kv.second;
		for (i = 0; i < seq.size(); ++i)
			if (seq[i].row > row)
				break;

		const Keypoint &prev = seq[i>0 ? i-1 : 0];
		const Keypoint &next = seq[i<seq.size() ? i : seq.size()-1];

		const float t = (prev.row != next.row) ?
			(row - prev.row) / (next.row - prev.row) : 0;

		const Value v = {
			prev.value.x + (next.value.x - prev.value.x) * t,
			prev.value.y + (next.value.y - prev.value.y) * t,
			prev.value.z + (next.value.z - prev.value.z) * t,
			prev.value.w + (next.value.w - prev.value.w) * t
		};
		slice.emplace(kv.first, v);
	}

	slice_ = std::move(slice);
}
