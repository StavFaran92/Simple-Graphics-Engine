#pragma once

#include <string>

#include "memory/AssetRef.h"

namespace cereal
{
	class JSONOutputArchive;
	class JSONInputArchive;
}

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

	// Concrete archive types because a cereal serialize template can't be virtual
	virtual void save(cereal::JSONOutputArchive& archive) const = 0;
	virtual void load(cereal::JSONInputArchive& archive) = 0;

	// The asset's data as cereal JSON; loadFromJson throws on bad data
	std::string saveToJson() const;
	void loadFromJson(const std::string& json);

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