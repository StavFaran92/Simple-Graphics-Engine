#pragma once

#include <string>

#include "memory/AssetRef.h"

class EngineAPI Asset
{
public:
	using ResourceType = Resource;

	//Asset(const AssetBuildDescriptor& desc);

	Asset() = default;

	virtual void bindDependency(const std::string& slot, UUID dependency) {}

	virtual void fillLoadDescriptor(ResourceLoadDescriptor& resourceLoadDesc) {};
	virtual void fillBuildDescriptor(ResourceBuildDescriptor& resourceBuildDesc) {};
	virtual void fillData(ResourceRef<Resource> resource) const {};

	virtual void serialize(nlohmann::json& j) const = 0;
	virtual void deserialize(const nlohmann::json& j) = 0;

	virtual std::vector<AssetRef<Asset>> gatherDependencies() const { return {}; };

	void syncAsset() const;

	void deleteAsset();

	void makeAssetDirty();

	const AssetRecord& info() const;

	ResourceRef<Resource> resource() const;

	inline bool isEmpty() const
	{
		return uuid == EMPTY_UUID;
	}

	UUID getUID() const;
	

protected:
	virtual std::string getRecommendedExtension(const AssetRecord& aInfo) { return getExtensionFromType(aInfo.aType); };
protected:
	UUID uuid;
	friend class Assets;
	friend void from_json(const nlohmann::json& j, AssetRecord& r);
	//AssetBuildDescriptor m_createDesc;
};

//CEREAL_REGISTER_TYPE(Asset);