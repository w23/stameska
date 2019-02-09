#include "PolledPipelineDesc.h"

#include "utils.h"

PolledPipelineDesc::PolledPipelineDesc(const std::shared_ptr<PolledFile> &file)
	: file_(file)
{
}

PolledPipelineDesc::~PolledPipelineDesc()
{
}

bool PolledPipelineDesc::poll(unsigned int poll_seq) {
	if (!beginUpdate(poll_seq))
		return false;

	if (!file_->poll(poll_seq))
		return false;

	auto result = renderdesc::Pipeline::load(std::string_view(
			(const char*)file_->data().data(),
			file_->data().size()));
	if (!result) {
		MSG("Error parsing pipeline desc: %s", result.error().c_str());
		return false;
	}

	pipeline_.reset(new renderdesc::Pipeline(std::move(result).value()));
	return endUpdate();
}
