#pragma once

#if !defined(_MSC_VER) || _MSC_VER >= 1910
#include <string_view>
using string_view = std::string_view;
#else

#include <string>
#include <cstring>

class string_view {
public:
	string_view(const char *str) : begin_(str), length_(strlen(str)) {}
	string_view(const char *str, size_t len) : begin_(str), length_(len) {}
	string_view(const std::string& str) : begin_(str.c_str()), length_(str.length()) {}

	const char *data() const { return begin_; }

	size_t size() const { return length_; }
	size_t length() const { return length_; }

	string_view substr(size_t begin, size_t len) const {
		if (begin >= length_)
			return string_view(nullptr, 0);

		const size_t size_left = length_ - begin;
		return string_view(begin_ + begin, size_left > len ? len : size_left);
	}

	std::string str() const { return std::string(begin_, length_); }
	operator std::string() const { return std::string(begin_, length_); }

	int compare(string_view other) const { return strncmp(begin_, other.begin_, length_ > other.length_ ? other.length_ : length_); }

	bool operator==(string_view other) const { return 0 == compare(other); }

	size_t find(const char *needle, size_t offset = 0) const {
		if (offset >= length_)
			return std::string::npos;

		// lol very naive
		for (; offset < length_; ++offset) {
			for (size_t i = 0;; ++i) {
				if (needle[i] == '\0')
					return offset;

				if (offset + i >= length_)
					return std::string::npos;
				
				if (begin_[offset + i] != needle[i])
					break;
			}
		}

		return std::string::npos;
	}

	struct ConstIterator {
		
		ConstIterator(const char *s) : s_(s) {}
		bool operator==(ConstIterator other) const { return s_ == other.s_; }
		ConstIterator& operator++() { s_++; return *this; }
		ConstIterator operator+(size_t i) const { return ConstIterator(s_ + i); }

		operator const char*() const { return s_;  }

	private:
		const char *s_;
	};

	ConstIterator cbegin() const { return ConstIterator(begin_); }
	ConstIterator cend() const { return ConstIterator(begin_ + length_); }

	char operator[](size_t i) const { return begin_[i]; }

private:
	const char *begin_;
	size_t length_;
};
#endif