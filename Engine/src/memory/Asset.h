#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "AssetFactory.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	Asset(const AssetCreateDescriptor& desc);

	virtual void bindDependency(const std::string& slot, UUID dependency) {}

protected:
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };

protected:
	AssetCreateDescriptor m_createDesc;
};
