#pragma once

#include "PolledResource.h"

#include "string_view.h"
#include <vector>

class PolledFile : public PolledResource {
public:
	PolledFile(string_view filename);
	~PolledFile();

	bool poll(unsigned int poll_seq);

	const std::vector<unsigned char>& data() const;

	string_view string() const {
		return string_view(reinterpret_cast<const char*>(data().data()), data().size());
	}

private:
	class Impl;
	std::unique_ptr<Impl> impl_;
};
