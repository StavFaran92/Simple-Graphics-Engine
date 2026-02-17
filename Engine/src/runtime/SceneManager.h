#pragma once

#include "memory/ResourcePipeline.h"

class SceneTypeManager : public ResourceTypeManager
{
public:
	ResourceWrapper<Resource> loadFromDisk(ResourceLoadDescriptor& desc) override;
	void saveAsset(const AssetRecord& record) override;
	void importAsset(const AssetRecord& record) override;
	Asset* createAsset(AssetCreateDescriptor& desc) override;
	ResourceLoadDescriptor* createLoadDescriptor(const AssetRecord& record) override;

	virtual void parse(ResourceLoadDescriptor& desc) override;
	virtual void parse(ResourceCreateDescriptor& desc) override;
};
