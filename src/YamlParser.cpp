#include "YamlParser.h"
#include "utils.h"

#include "yaml.h"
#include <stdio.h>
#include <stdexcept>
#include <memory>

#ifndef DEBUG
#undef MSG
#define MSG(...)
#endif

namespace yaml {

class ParserContext {
	yaml_parser_t parser_;

	struct YamlEvent {
		yaml_event_type_t type;
		std::string value;

		YamlEvent(const yaml_event_t &event)
			: type(event.type)
			, value(event.type == YAML_SCALAR_EVENT ? (const char*)event.data.scalar.value : "")
		{
		}
	};

	YamlEvent getNextEvent() {
		yaml_event_t event;
		if (!yaml_parser_parse(&parser_, &event)) {
			throw std::runtime_error(format("Error parsing yaml %d@%d: %s",
				(int)parser_.problem_mark.line, (int)parser_.problem_mark.column,
				parser_.problem));
		}
		using YamlEventPtr = std::unique_ptr<yaml_event_t, decltype(&yaml_event_delete)>;
		const auto event_holder = YamlEventPtr(&event, &yaml_event_delete);

		return YamlEvent(event);
	}

	Sequence readSequence() {
		Sequence seq;
		for (;;) {
			YamlEvent event = getNextEvent();

			switch (event.type) {
				case YAML_SCALAR_EVENT:
					MSG("YAML_SCALAR_EVENT: value=%s", event.value.c_str());
					seq.emplace_back(std::move(event.value));
					break;
				case YAML_SEQUENCE_START_EVENT:
					MSG("YAML_SEQUENCE_START_EVENT");
					seq.emplace_back(readSequence());
					break;
				case YAML_MAPPING_START_EVENT:
					MSG("YAML_MAPPING_START_EVENT");
					seq.emplace_back(readMapping());
					break;
				case YAML_SEQUENCE_END_EVENT:
					MSG("YAML_SEQUENCE_END_EVENT");
					return seq;
				default:
					throw std::runtime_error(format("Unexpected event %d", event.type));
			}
		}
	}

	Mapping readMapping() {
		Mapping mapping;
		for (;;) {
			std::string key;
			{
				YamlEvent event = getNextEvent();

				switch (event.type) {
					case YAML_SCALAR_EVENT:
						MSG("YAML_SCALAR_EVENT: value=%s", event.value.c_str());
						key = std::move(event.value);
						break;
					case YAML_MAPPING_END_EVENT:
						MSG("YAML_MAPPING_END_EVENT");
						return mapping;
					default:
						throw std::runtime_error(format("Unexpected event %d", event.type));
					}
			}

			{
				YamlEvent event = getNextEvent();
				
				switch (event.type) {
					case YAML_SCALAR_EVENT:
						MSG("YAML_SCALAR_EVENT: value=%s", event.value.c_str());
						mapping.map_.emplace(std::move(key), std::move(event.value));
						break;
					case YAML_SEQUENCE_START_EVENT:
						MSG("YAML_SEQUENCE_START_EVENT");
						mapping.map_.emplace(std::move(key), readSequence());
						break;
					case YAML_MAPPING_START_EVENT:
						MSG("YAML_MAPPING_START_EVENT");
						mapping.map_.emplace(std::move(key), readMapping());
						break;
					default:
						throw std::runtime_error(format("Unexpected event %d", event.type));
				}
			}
		}
	}

public:
	ParserContext(FILE &f) {
		yaml_parser_initialize(&parser_);
		yaml_parser_set_input_file(&parser_, &f);
	}

	ParserContext(std::string_view s) {
		yaml_parser_initialize(&parser_);
		yaml_parser_set_input_string(&parser_, (const unsigned char*)s.data(), s.size());
	}

	~ParserContext() {
		yaml_parser_delete(&parser_);
	}

	Value readAnyValue() {
		enum class State {
			Init,
			StreamStarted,
			DocumentStarted,
		};

		for(State state = State::Init; state != State::DocumentStarted;) {
			const YamlEvent event = getNextEvent();

			switch (event.type) {
				case YAML_STREAM_START_EVENT:
					if (state != State::Init)
						throw std::runtime_error("Unexpected YAML_STREAM_START_EVENT");
					state = State::StreamStarted;
					break;
				case YAML_DOCUMENT_START_EVENT:
					if (state != State::StreamStarted)
						throw std::runtime_error("Unexpected YAML_DOCUMENT_START_EVENT");
					state = State::DocumentStarted;
					break;
				default:
					throw std::runtime_error(format("Unexpected event %d", event.type));
			} // switch (event.type)
		} // for(state != DocumentStarted)

		YamlEvent event = getNextEvent();

		switch (event.type) {
			case YAML_SCALAR_EVENT:
				MSG("YAML_SCALAR_EVENT: value=%s", event.value.c_str());
				return Value(std::move(event.value));
			case YAML_SEQUENCE_START_EVENT:
				MSG("YAML_SEQUENCE_START_EVENT");
				return Value(readSequence());
			case YAML_MAPPING_START_EVENT:
				MSG("YAML_MAPPING_START_EVENT");
				return Value(readMapping());
			default:
				throw std::runtime_error(format("Unexpected event %d", event.type));
		}
	}
};

Value parse(const char *filename) {
	MSG("Parsing YAML from %s", filename);

	const auto f = std::unique_ptr<FILE, decltype(&fclose)>(fopen(filename, "rb"), &fclose);

	if (!f) {
		const int err = errno;
		char strerr[32] = {'\0'};
		const auto v = strerror_r(err, strerr, sizeof(strerr));
		(void)v;
		throw std::runtime_error(format("Cannot open file %s: %d(%s)", filename, err, strerr));
	}

	ParserContext ctx(*f);

	return ctx.readAnyValue();
}

Value parse(std::string_view s) {
	ParserContext ctx(s);

	return ctx.readAnyValue();
}

const Value &Mapping::getValue(const std::string &name) const {
	const auto it = map_.find(name);
	if (it == map_.end())
		throw std::runtime_error(format("Field %s not found", name.c_str()));

	return it->second;
}

const Mapping &Mapping::getMapping(const std::string &name) const {
	return getValue(name).getMapping();
}

const Sequence &Mapping::getSequence(const std::string &name) const {
	return getValue(name).getSequence();
}

const std::string &Mapping::getString(const std::string &name) const {
	return getValue(name).getString();
}

int Mapping::getInt(const std::string &name) const {
	return getValue(name).getInt();
}

const std::string &Mapping::getString(const std::string &name, const std::string &default_value) const {
	const auto it = map_.find(name);
	if (it == map_.end())
		return default_value;

	return it->second.getString();
}

int Mapping::getInt(const std::string &name, int default_value) const {
	const auto it = map_.find(name);
	if (it == map_.end())
		return default_value;

	return it->second.getInt();
}

} // namespace yaml
