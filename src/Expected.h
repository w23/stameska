#pragma once

#include <utility>
#include <stdexcept>

template <typename E>
class Unexpected {
public:
	Unexpected() = delete;
	explicit Unexpected(const E &e) : error_(e) {}
	explicit Unexpected(E &&e) : error_(std::move(e)) {}

	const E &get() const & { return error_; }
	E &&get() const && { return std::move(error_); }

private:
	E error_;
};

template <typename T, typename E>
class [[nodiscard]] Expected {
public:
	using UnexpectedType = Unexpected<E>;

	Expected() : value_(T()), has_value_(true) {}
	Expected(const T& value) : value_(value), has_value_(true) {}
	Expected(T &&value) : value_(std::move(value)), has_value_(true) {}
	Expected(UnexpectedType &&unexpected) : error_(std::move(unexpected)), has_value_(false) {}

	~Expected() {
		if (has_value_)
			value_.~T();
		else
			error_.~UnexpectedType();
	}

	bool hasValue() const { return has_value_; }
	const E& error() const & {
		if (has_value_)
			throw std::runtime_error("Expected has value");
		return error_.get();
	}

	E&& error() const && {
		if (has_value_)
			throw std::runtime_error("Expected has value");
		return std::move(error_.get());
	}

	const T& value() const & {
		if (!has_value_)
			throw error_.get();
		return value_;
	}

	T&& value() && {
		if (!has_value_)
			throw error_.get();
		return std::move(value_);
	}

	const T *operator->() const {
		if (!has_value_)
			throw error_.get();
		return &value_;
	}

	explicit operator bool() const { return hasValue(); }

private:
	union {
		T value_;
		UnexpectedType error_;
	};

	bool has_value_;
};

template <typename E>
class [[nodiscard]] Expected<void, E> {
public:
	using UnexpectedType = Unexpected<E>;

	Expected() : has_value_(true) {}
	Expected(UnexpectedType &&unexpected) : error_(std::move(unexpected)), has_value_(false) {}

	~Expected() {
		if (!has_value_)
			error_.~UnexpectedType();
	}

	bool hasValue() const { return has_value_; }

	const E& error() const & {
		if (has_value_)
			throw std::runtime_error("Expected has value");
		return error_.get();
	}

	E&& error() const && {
		if (has_value_)
			throw std::runtime_error("Expected has value");
		return std::move(error_.get());
	}

	explicit operator bool() const { return hasValue(); }

private:
	union {
		UnexpectedType error_;
	};
	bool has_value_;
};
