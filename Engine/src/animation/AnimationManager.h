#pragma once

#include "memory/ResourcePipeline.h"

class AnimationTypeManager : public ResourceTypeManager
{
public:

	// Inherited via ResourceTypeManager
	Ref<Asset> createAsset(AssetCreateDescriptor& desc) override;
	bool importAsset(const std::string& src, ImportNode& result) override;
	bool saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst) override;
	ResourceLoadDescriptor* makeResourceLoadDescriptor() override;
	ResourceWrapper<Resource> loadResourceFromDisk(ResourceLoadDescriptor& desc) override;
	void parse(ResourceLoadDescriptor& desc) override;
	void parse(ResourceCreateDescriptor& desc) override;
};
