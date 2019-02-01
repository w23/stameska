#pragma once

#include "PolledResource.h"

#include <string_view>
#include <vector>

class PolledFile : public PolledResource {
public:
	PolledFile(std::string_view filename);
	~PolledFile();

	bool poll(unsigned int poll_seq);

	const std::vector<unsigned char>& data() const;

	std::string_view string() const {
		return std::string_view(reinterpret_cast<const char*>(data().data()), data().size());
	}

private:
	class Impl;
	std::unique_ptr<Impl> impl_;
};
