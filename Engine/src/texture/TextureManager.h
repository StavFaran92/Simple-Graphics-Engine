#pragma once

#include "memory/ResourcePipeline.h"

class TextureTypeManager : public ResourceTypeManager
{
public:

	// Inherited via ResourceTypeManager
	Ref<Asset> createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc) override;

	Ref<Asset> deserializeAsset(const nlohmann::json& j) override;

	bool importAsset(const std::string& src, ImportNode& result) override;

	bool saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst) override;

	ResourceLoadDescriptor* makeResourceLoadDescriptor() override;

	ResourceWrapper<Resource> loadResourceFromDisk(ResourceLoadDescriptor& desc) override;

	void parse(ResourceLoadDescriptor& desc) override;

	void parse(ResourceBuildDescriptor& desc) override;
};
