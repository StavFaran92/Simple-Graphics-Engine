#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "AssetFactory.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	//Asset(const AssetCreateDescriptor& desc);

	Asset() = default;

	void updateAsset(AssetUpdateDescriptor desc);

	virtual void bindDependency(const std::string& slot, UUID dependency) {}

	virtual void fillData(ResourceWrapper<Resource> resource) {};

	virtual void serialize(nlohmann::json& j) const = 0;
	virtual void deserialize(const nlohmann::json& j) = 0;

protected:
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };
protected:
	UUID uuid;
	//AssetCreateDescriptor m_createDesc;
};

//CEREAL_REGISTER_TYPE(Asset);