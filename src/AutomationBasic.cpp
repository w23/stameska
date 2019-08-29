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

		Variable variable = {0, {}};

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

			if (yrow.size() > 5) {
				MSG("Error parsing %s row: too many components %d, max 4", var.c_str(), static_cast<int>(yrow.size()));
				return;
			}

			const int yrow_components = static_cast<int>(yrow.size() - 1);
			if (variable.components == 0) {
				variable.components = yrow_components;
			} else if (variable.components != yrow_components) {
				MSG("Error parsing %s row: inconsistent number of components %d, %d expected",
					var.c_str(), yrow_components, variable.components);
				return;
			}

			Keypoint kp = {0,{0,0,0,0}};
			READ_FLOAT(0, time, kp.row);
			READ_FLOAT(1, x, kp.value.x);
			if (yrow.size() > 2) READ_FLOAT(2, y, kp.value.y);
			if (yrow.size() > 3) READ_FLOAT(3, z, kp.value.z);
			if (yrow.size() > 4) READ_FLOAT(4, w, kp.value.w);

			if (!variable.sequence.empty() && variable.sequence.back().row > kp.row) {
				MSG("Error parsing %s rows: monotonic time expected", var.c_str());
				return;
			}

			variable.sequence.push_back(kp);
		}

		if (!variable.sequence.empty())
			new_data.emplace(std::move(var), std::move(variable));
	}

	data_ = std::move(new_data);
}

void AutomationBasic::update(float row) {
	reread();

	Slice slice;
	for (const auto &kv: data_) {
		size_t i;
		const Variable &var = kv.second;
		for (i = 0; i < var.sequence.size(); ++i)
			if (var.sequence[i].row > row)
				break;

		const Keypoint &prev = var.sequence[i>0 ? i-1 : 0];
		const Keypoint &next = var.sequence[i<var.sequence.size() ? i : var.sequence.size()-1];

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

Expected<IAutomation::ExportResult, std::string> AutomationBasic::writeExport(std::string_view config, const shader::UniformsMap &uniforms) const {
	if (config != "C")
		return Unexpected<std::string>(format("Export config '%*.s' not implemented", PRISV(config)));

	ExportResult result;

	std::vector<int> lengths_table;
	std::vector<int> times_table;
	std::vector<int> values_table;

	std::vector<float> time_data;
	std::vector<float> value_data;

	int uniform_block_offset = 0;
	for (const auto &[n, u]: uniforms) {
		const auto it = data_.find(n);
		if (it == data_.end())
			return Unexpected(format("No automation for uniform '%s'", n.c_str()));

		if (it->second.components != static_cast<int>(u.type))
			return Unexpected(format("Uniform '%s' component count mismatch (%d vs %d)", n.c_str(), it->second.components, static_cast<int>(u.type)));

		// Don't write if its constant
		if (it->second.sequence.size() == 1) {
			result.uniforms.insert({n, it->second.sequence[0].value});
			continue;
		}

		// Write output offset in uniform block
		result.uniforms.insert({n, uniform_block_offset});
		uniform_block_offset += it->second.components;

		// Write lengths
		for (int i = 0; i < it->second.components; ++i)
			lengths_table.emplace_back(static_cast<int>(it->second.sequence.size()));

		// Write time table offsets
		for (int i = 0; i < it->second.components; ++i)
			times_table.emplace_back(static_cast<int>(time_data.size()));

		// Write time table (delta)
		{
			float prev_time = 0.f;
			for (const auto &kv: it->second.sequence) {
				time_data.emplace_back(kv.row - prev_time);
				prev_time = kv.row;
			}
		}

		// Write values
		for (int i = 0; i < it->second.components; ++i) {
			values_table.emplace_back(static_cast<int>(value_data.size()));
			for (const auto &kv: it->second.sequence) {
				value_data.emplace_back((&kv.value.x)[i]);
			}
		}
	}

	// Make sections
	result.buffer_size = uniform_block_offset;
	const std::string head_str = format("#define AUTOMATION_SIZE %d\n", result.buffer_size);
	result.sections.emplace_back(Section{Section::Type::Data, "automation_meta", "Constant",
		std::vector<char>(head_str.begin(), head_str.end())});

	// Lengths table
	std::string lengths_table_str = "static const int automation_lengths_table[AUTOMATION_SIZE] = {\n";
	for (const auto &it: lengths_table)
		lengths_table_str += format("\t%d,\n", it);
	lengths_table_str += "};\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_lengths_table", "",
		std::vector<char>(lengths_table_str.begin(), lengths_table_str.end())});

	// Times table
	std::string times_table_str = "static const int automation_times_table[AUTOMATION_SIZE] = {\n";
	for (const auto &it: times_table)
		times_table_str += format("\t%d,\n", it);
	times_table_str += "};\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_times_table", "",
		std::vector<char>(times_table_str.begin(), times_table_str.end())});

	// Times table
	std::string values_table_str = "static const int automation_values_table[AUTOMATION_SIZE] = {\n";
	for (const auto &it: values_table)
		values_table_str += format("\t%d,\n", it);
	values_table_str += "};\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_values_table", "",
		std::vector<char>(values_table_str.begin(), values_table_str.end())});

	// Time data
	std::string time_data_str = "static const float automation_time_data[] = {\n";
	for (const auto &it: time_data)
		time_data_str += format("\t%f,\n", it);
	time_data_str += "};\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_time_data", "",
		std::vector<char>(time_data_str.begin(), time_data_str.end())});

	// Value data
	std::string value_data_str = "static const float automation_value_data[] = {\n";
	for (const auto &it: value_data)
		value_data_str += format("\t%f,\n", it);
	value_data_str += "};\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_value_data", "",
		std::vector<char>(value_data_str.begin(), value_data_str.end())});

	// Include player
	const char include_player[] = "#include \"stameska_automation.h\"\n";
	result.sections.emplace_back(Section{Section::Type::Data, "automation_player", "",
		std::vector<char>(include_player, include_player + sizeof(include_player)-1)});

	return result;
}
