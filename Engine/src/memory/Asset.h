#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "AssetFactory.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	Asset(const AssetCreateDescriptor& desc);

	const AssetCreateDescriptor& getDescriptor() const;

	UUID getUUID() const;

	virtual void fillData(ResourceLoadDescriptor& loadDesc) const {};
protected:
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };

	

protected:
	AssetCreateDescriptor m_createDesc;

	UUID m_uuid = EMPTY_UUID;
};