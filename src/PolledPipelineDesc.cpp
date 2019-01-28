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

	try {
		pipeline_.reset(
			new renderdesc::Pipeline(string_view(
				(const char*)file_->data().data(),
				file_->data().size())));
		return endUpdate();
	} catch (const std::exception &e) {
		MSG("Error parsing render desc: %s", e.what());
		return false;
	}
}
