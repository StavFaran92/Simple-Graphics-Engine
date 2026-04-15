#pragma once

#include <string>

#include "memory/AssetHandle.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	//Asset(const AssetBuildDescriptor& desc);

	Asset() = default;

	virtual void bindDependency(const std::string& slot, UUID dependency) {}

	virtual void fillLoadDescriptor(ResourceLoadDescriptor& resourceLoadDesc) {};
	virtual void fillBuildDescriptor(ResourceBuildDescriptor& resourceBuildDesc) {};
	virtual void fillData(ResourceWrapper<Resource> resource) {};

	virtual void serialize(nlohmann::json& j) const = 0;
	virtual void deserialize(const nlohmann::json& j) = 0;

protected:
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };
protected:
	UUID uuid;
	//AssetBuildDescriptor m_createDesc;
};

//CEREAL_REGISTER_TYPE(Asset);