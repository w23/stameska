#pragma once
#include "Expected.h"
#include "utils.h"

#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <functional>

template <typename T, typename E>
using ExpectedRef = Expected<std::reference_wrapper<T>, E>;

namespace yaml {

class Value;
using Sequence = std::vector<Value>;

class ParserContext;

class Mapping {
public:
	bool hasKey(const std::string &name) const;

	using KeyValue = std::map<std::string, Value>;
	const KeyValue &map() const { return map_; }

	ExpectedRef<const Mapping, std::string> getMapping(const std::string &name) const;
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

	ExpectedRef<const Value, std::string> getValue(const std::string &name) const;

	KeyValue map_;
};

class Value {
public:
	ExpectedRef<const Mapping, std::string> getMapping() const {
		if (type_ != Type::Mapping)
			return Unexpected<std::string>("Value is not of Mapping type");
		return std::cref(mapping_);
	}

	const Sequence &getSequence() const {
		if (type_ != Type::Sequence)
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

		return intFromString(string_);
	}

	bool isString() const { return type_ == Type::String; }

	Value(std::string &&s) : type_(Type::String), string_(std::move(s)) {}
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

Expected<Value, std::string> parse(const char *filename);
Expected<Value, std::string> parse(std::string_view s);

} // namespace yaml

