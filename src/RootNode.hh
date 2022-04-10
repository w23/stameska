#pragma once

#include "INode.hh"
#include "ProjectSettings.h"
#include "AudioCtl.h"
#include "VideoNode.h"

#include "filesystem.h"

class RootNode : public INode {
public:
	RootNode(const fs::path &root, const ProjectSettings &settings);
	~RootNode();

	void resize(int w, int h);
	void paint(ATimeUs ts, float dt);
	void key(ATimeUs ts, AKey key, int down);
	void pointer(ATimeUs ts, int dx, int dy, unsigned int dbtn);

	virtual void visitChildren(const std::function<void(INode*)>& visitor) noexcept override;

private:
	static void nodeTreeFunc(INode* node);

	static std::string root_node_class_name_;
	std::unique_ptr<IAutomation> automation_;
	AudioCtl audio_;
	VideoNode video_;

	struct {
		ATimeUs last_print = 0;
		int frames = 0;
	} fpstat;
};
