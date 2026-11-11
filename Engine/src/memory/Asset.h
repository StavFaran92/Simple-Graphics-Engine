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

	// Metadata: settings about the asset's file, kept in its registry record (e.g. a shader override)
	virtual void saveMetadata(cereal::JSONOutputArchive& archive) const {}
	virtual void loadMetadata(cereal::JSONInputArchive& archive) {}

	// Data the asset holds itself and the user edits (a material's MaterialData); undo snapshots it with the metadata
	virtual void serializeData(cereal::JSONOutputArchive& archive) const {}
	virtual void deserializeData(cereal::JSONInputArchive& archive) {}

	// As cereal JSON; the From functions throw on bad data
	std::string metadataToJson() const;
	void metadataFromJson(const std::string& json);
	std::string dataToJson() const;
	void dataFromJson(const std::string& json);

	// Reads what the asset needs from its own file when the project loads
	virtual void loadFromFile(const std::string& path) {}

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