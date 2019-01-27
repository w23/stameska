#pragma once

#include "PolledResource.h"
#include "PolledFile.h"
#include "RenderDesc.h"

#include <memory>

class PolledPipelineDesc : public PolledResource {
public:
	PolledPipelineDesc(const std::shared_ptr<PolledFile> &file);
	~PolledPipelineDesc();

	bool poll(unsigned int poll_seq);

	const std::shared_ptr<renderdesc::Pipeline> &get() const { return pipeline_; }
	const std::shared_ptr<renderdesc::Pipeline> &operator->() const { return pipeline_; }

private:
	const std::shared_ptr<PolledFile> file_;

	std::shared_ptr<renderdesc::Pipeline> pipeline_;
};
