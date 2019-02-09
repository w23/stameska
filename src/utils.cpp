#include "utils.h"

#include <stdexcept>
#include <cstdarg>

std::string format(const char* fmt, ...) {
	va_list args, args_len;
	va_start(args, fmt);
	va_copy(args_len, args);
	const int len = vsnprintf(nullptr, 0, fmt, args_len);
	va_end(args_len);

	std::string ret = std::string(len, ' ');
	vsnprintf(&ret.front(), len + 1, fmt, args);
	va_end(args);

	return ret;
}

Expected<long int,std::string> intFromString(const std::string &s) {
		char *endptr = nullptr;
		const long int ret = strtol(s.c_str(), &endptr, 10);
		if (s.empty() || endptr[0] != '\0')
			return Unexpected(format("Cannot convert '%s' to int", s.c_str()));
		return ret;
}
