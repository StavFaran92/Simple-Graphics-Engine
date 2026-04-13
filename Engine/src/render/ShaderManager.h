#pragma once

#include "memory/ResourcePipeline.h"

class ShaderTypeManager : public ResourceTypeManager
{
public:


	// Inherited via ResourceTypeManager
	Ref<Asset> createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc) override;

	Ref<Asset> deserializeAsset(const nlohmann::json& j) override;

	bool importAsset(const ResourceLoadDescriptor& loadDesc, ImportNode& result) override;

	bool saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst) override;

	std::unique_ptr<ResourceLoadDescriptor> makeResourceLoadDescriptor() override;
	std::unique_ptr<ResourceBuildDescriptor> makeResourceBuildDescriptor() override;

	ResourceWrapper<Resource> loadResourceFromDisk(ResourceLoadDescriptor& desc) override;

	void parse(ResourceLoadDescriptor& desc) override;

	void parse(ResourceBuildDescriptor& desc) override;

};
