#pragma once

#include <string>

#include "memory/AssetHandle.h"
#include "AssetFactory.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	Asset(const AssetCreateDescriptor& desc);

	void updateAsset(const AssetUpdateDescriptor& uDesc);

	virtual void save(const AssetRecord& aInfo) {}

	virtual void bindDependency(const std::string& slot, UUID dependency) {}

	const AssetCreateDescriptor& getDescriptor() const;

	UUID getUUID() const;

protected:
	virtual bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) { return false; }

	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };

protected:
	AssetCreateDescriptor m_createDesc;

	UUID m_uuid = EMPTY_UUID;
};
