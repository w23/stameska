#pragma once
#include "utils.h"

#include <vector>
#include <map>
#include <string>

namespace yaml {

class Value;
using Sequence = std::vector<Value>;

class ParserContext;

class Mapping {
public:
	const Mapping &getMapping(const std::string &name) const;
	const Sequence &getSequence(const std::string &name) const;
	const std::string &getString(const std::string &name) const;
	int getInt(const std::string &name) const;
	const std::string &getString(const std::string &name, const std::string &default_value) const;
	int getInt(const std::string &name, int default_value) const;

	Mapping() = default;
	Mapping(Mapping &&) = default;
	Mapping(const Mapping&) = delete;

	Mapping &operator=(const Mapping &) = delete;

private:
	friend class ParserContext;

	const Value &getValue(const std::string &name) const;

	std::map<std::string, Value> map_;
};

class Value {
public:
	const Mapping &getMapping() const {
		if (type_ != Type::Mapping)
			throw std::runtime_error("Value is not of Mapping type");
		return mapping_;
	}

	const Sequence &getSequence() const {
		if (type_ != Type::Mapping)
			throw std::runtime_error("Value is not of Sequence type");
		return sequence_;
	}

	const std::string &getString() const {
		if (type_ != Type::String)
			throw std::runtime_error("Value is not of String type");
		return string_;
	}

	int getInt() const {
		if (type_ != Type::String)
			throw std::runtime_error("Value is not of String type");

		char *endptr = nullptr;
		const long int ret = strtol(string_.c_str(), &endptr, 10);
		if (string_.empty() || endptr[0] != '\0')
			throw std::runtime_error(format("Cannot convert '%string_' to int", string_.c_str()));

		return ret;
	}

	Value(const char *s) : type_(Type::String), string_(s) {}
	Value(Mapping &&mapping) : type_(Type::Mapping), mapping_(std::move(mapping)) {}
	Value(Sequence &&sequence) : type_(Type::Sequence), sequence_(std::move(sequence)) {}

private:
	friend class ParserContext;

	enum class Type {
		String, Mapping, Sequence
	};

	Type type_;
	std::string string_;
	Mapping mapping_;
	Sequence sequence_;
};

Value parse(const char *filename);

} // namespace yaml

