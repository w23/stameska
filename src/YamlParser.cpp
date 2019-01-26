#include "YamlParser.h"
#include "utils.h"

#include "yaml.h"
#include <stdio.h>
#include <stdexcept>
#include <memory>

namespace yaml {

class ParserContext {
#if 0
	std::vector<yaml::Value> stack_;

	void doEnd() {
		popContext();
	}

	void popContext() {
		if (context_stack.empty())
			throw std::runtime_error("Context stack underflow");
		context_stack.pop_back();
	}

public:
	ParserContext(yaml::Value &value) : context_stack({&top_context}) {}
	~ParserContext()
	{
		/*
		if (!context_stack.empty())
			throw std::runtime_error("Unbalanced context stack");
		*/
	}

	const Context *top() const {
		if (context_stack.empty())
			throw std::runtime_error("Context context stack is empty");

		return context_stack.back();
	}

	void handleSequenceStart() {
		doAction(top()->sequence.action);
	}

	void handleSequenceEnd() {
		doEnd();
	}

	void handleMappingStart() {
		doAction(top()->mapping.action);
	}

	void handleMappingEnd() {
		doEnd();
	}

	void handleScalar(const char *scalar) {
		const Context *c = top();
		const auto it = c->scalar.map.find(scalar);
		doAction((it == c->scalar.map.end()) ? c->scalar.default_action : it->second, scalar);
		popContext();
	}
#endif

	using YamlEventPtr = std::unique_ptr<yaml_event_t, decltype(&yaml_event_delete)>;

	yaml_parser_t &parser_;

	void getNextEvent(yaml_event_t &event) {
		if (!yaml_parser_parse(&parser_, &event)) {
			throw std::runtime_error(format("Error parsing yaml %d@%d: %s",
				(int)parser_.problem_mark.line, (int)parser_.problem_mark.column,
				parser_.problem));
		}
	}

	Sequence readSequence() {
		Sequence seq;
		for (;;) {
			yaml_event_t event;
			getNextEvent(event);
			const auto event_holder = YamlEventPtr(&event, &yaml_event_delete);

			switch (event.type) {
				case YAML_SCALAR_EVENT:
					MSG("YAML_SCALAR_EVENT: value=%s length=%d style=%d",
						event.data.scalar.value,
						(int)event.data.scalar.length, event.data.scalar.style);
					seq.emplace_back((const char*)event.data.scalar.value);
					break;
				case YAML_SEQUENCE_START_EVENT:
					MSG("YAML_SEQUENCE_START_EVENT: anchor=%s",
							event.data.sequence_start.anchor);
					seq.emplace_back(readSequence());
					break;
				case YAML_MAPPING_START_EVENT:
					MSG("YAML_MAPPING_START_EVENT: anchor=%s",
							event.data.mapping_start.anchor);
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
				yaml_event_t event;
				getNextEvent(event);
				const auto event_holder = YamlEventPtr(&event, &yaml_event_delete);

				switch (event.type) {
					case YAML_SCALAR_EVENT:
						MSG("YAML_SCALAR_EVENT: value=%s length=%d style=%d",
							event.data.scalar.value,
							(int)event.data.scalar.length, event.data.scalar.style);
						key = (const char*)event.data.scalar.value;
						break;
					case YAML_MAPPING_END_EVENT:
						MSG("YAML_MAPPING_END_EVENT");
						return mapping;
					default:
						throw std::runtime_error(format("Unexpected event %d", event.type));
					}
			}

			{
				yaml_event_t event;
				getNextEvent(event);
				const auto event_holder = YamlEventPtr(&event, &yaml_event_delete);
				
				switch (event.type) {
					case YAML_SCALAR_EVENT:
						MSG("YAML_SCALAR_EVENT: value=%s length=%d style=%d",
							event.data.scalar.value,
							(int)event.data.scalar.length, event.data.scalar.style);
						mapping.map_.emplace(std::move(key), (const char*)event.data.scalar.value);
						break;
					case YAML_SEQUENCE_START_EVENT:
						MSG("YAML_SEQUENCE_START_EVENT: anchor=%s",
								event.data.sequence_start.anchor);
						mapping.map_.emplace(std::move(key), readSequence());
						break;
					case YAML_MAPPING_START_EVENT:
						MSG("YAML_MAPPING_START_EVENT: anchor=%s",
								event.data.mapping_start.anchor);
						mapping.map_.emplace(std::move(key), readMapping());
						break;
					default:
						throw std::runtime_error(format("Unexpected event %d", event.type));
				}
			}
		}
	}

public:
	ParserContext(yaml_parser_t &parser) : parser_(parser) {
	}

	Value readAnyValue() {
		yaml_event_t event;
		getNextEvent(event);
		const auto event_holder = YamlEventPtr(&event, &yaml_event_delete);

		switch (event.type) {
			case YAML_SCALAR_EVENT:
				MSG("YAML_SCALAR_EVENT: value=%s length=%d style=%d",
					event.data.scalar.value,
					(int)event.data.scalar.length, event.data.scalar.style);
				return Value((const char*)event.data.scalar.value);
			case YAML_SEQUENCE_START_EVENT:
				MSG("YAML_SEQUENCE_START_EVENT: anchor=%s",
						event.data.sequence_start.anchor);
				return Value(readSequence());
			case YAML_MAPPING_START_EVENT:
				MSG("YAML_MAPPING_START_EVENT: anchor=%s",
						event.data.mapping_start.anchor);
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

	yaml_parser_t parser;

	// wat
	const auto parser_holder =
		std::unique_ptr<yaml_parser_t, decltype(&yaml_parser_delete)>(
			&parser, &yaml_parser_delete);

	yaml_parser_initialize(&parser);
	yaml_parser_set_input_file(&parser, f.get());

	enum class State {
		Init,
		StreamStarted,
		DocumentStarted,
		DocumentEnded,
		StreamEnded
	};

	ParserContext ctx(parser);

	for(State state = State::Init; state != State::DocumentStarted;) {//StreamEnded;) {
		yaml_event_t event;
		if (!yaml_parser_parse(&parser, &event)) {
			throw std::runtime_error(format("Error parsing yaml %s:%d@%d: %s",
				filename, (int)parser.problem_mark.line, (int)parser.problem_mark.column,
				parser.problem));
		}

		const auto event_holder =
			std::unique_ptr<yaml_event_t, decltype(&yaml_event_delete)>(
				&event, &yaml_event_delete);

		switch (event.type) {
			case YAML_NO_EVENT:
				throw std::runtime_error("YAML_NO_EVENT");
			case YAML_STREAM_START_EVENT:
				if (state != State::Init)
					throw std::runtime_error("Unexpected YAML_STREAM_START_EVENT");
				state = State::StreamStarted;
				break;
			case YAML_STREAM_END_EVENT:
				if (state != State::DocumentEnded)
					throw std::runtime_error("Unexpected YAML_STREAM_END_EVENT");
				state = State::StreamEnded;
				break;
			case YAML_DOCUMENT_START_EVENT:
				if (state != State::StreamStarted)
					throw std::runtime_error("Unexpected YAML_DOCUMENT_START_EVENT");
				state = State::DocumentStarted;
				break;
			case YAML_DOCUMENT_END_EVENT:
				if (state != State::DocumentStarted)
					throw std::runtime_error("Unexpected YAML_DOCUMENT_END_EVENT");
				state = State::DocumentEnded;
				break;
			case YAML_ALIAS_EVENT:
				throw std::runtime_error(
					format("Unsupported YAML_ALIAS_EVENT: anchor=%s",
						event.data.alias.anchor));
				break;
			default:
				throw std::runtime_error(format("Unexpected event %d", event.type));
		} // switch (event.type)
	} // for(state != StreamEnded)

	return ctx.readAnyValue();
}

/*
template <>
const Mapping &Value::get() const {
	if (type_ != Type::Mapping)
		throw std::runtime_error("Value is not of Mapping type");
	return mapping_;
}

template <>
const Sequence &Value::get() const {
	if (type_ != Type::Mapping)
		throw std::runtime_error("Value is not of Sequence type");
	return sequence_;
}
*/

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
