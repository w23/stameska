#pragma once

#include <memory>
#include <string_view>

class PolledResource {
public:
	unsigned int version() const { return version_; }

protected:
	bool beginUpdate(unsigned int poll_seq) {
		if (poll_seq == poll_seq_)
			return false;

		poll_seq_ = poll_seq;
		return true;
	}

	bool endUpdate() {
		++version_;
		return true;
	}

private:
	unsigned int poll_seq_ = 0;
	unsigned int version_ = 0;
};

template <typename R>
class PollMux : public PolledResource {
public:
	PollMux(const std::shared_ptr<R>& resource) : resource_(resource) {}

	bool poll(unsigned int poll_seq) {
		if (beginUpdate(poll_seq) && (resource_->poll(poll_seq) || resource_->version() != last_version_)) {
			last_version_ = resource_->version();
			return endUpdate();
		}

		return false;
	}

	const R* operator->() const { return resource_.get(); }

private:
	unsigned int last_version_ = 0;
	std::shared_ptr<R> resource_;
};
