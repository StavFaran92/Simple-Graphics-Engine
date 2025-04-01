#pragma once

#include <vector>

#include <nlohmann/json.hpp>
#include "UUID.h"

#include "Resource.h"
#include "Texture.h"

class ProjectAssetRegistry
{
public:
	struct TextureAsset
	{
		UUID uuid;
		std::string ext;

		// Friend function for to_json
		friend void to_json(nlohmann::json& j, const TextureAsset& asset) {
			j = nlohmann::json{
				{"uuid", asset.uuid},
				{"ext", asset.ext}
			};
		}

		// Friend function for from_json
		friend void from_json(const nlohmann::json& j, TextureAsset& asset) {
			j.at("uuid").get_to(asset.uuid);
			j.at("ext").get_to(asset.ext);
		}
	};
	ProjectAssetRegistry(const std::string& filename);
	static std::shared_ptr<ProjectAssetRegistry> create(const std::string& filename);
	static std::shared_ptr<ProjectAssetRegistry> parse(const std::string& filename);

	std::string getFilepath() const;
	void reset();
	void load(); //????
	void save();
	void addMesh(UUID uuid);
	void addTexture(Resource<Texture> texture);
	void addAnimation(UUID uuid);
	void addShader(UUID uuid);
	void addAssetRegistry(UUID uuid, AssetType aType);
	void addAssociation(std::string name, UUID uuid);
	std::vector<UUID> getMeshList() const;
	std::vector<TextureAsset> getTextureList() const;
	std::vector<UUID> getAnimationList() const;
	std::unordered_map<std::string, UUID> getAssociations() const;


private:
	
	void sync();

private:
	std::string m_filename;
	nlohmann::json m_assetRegistry;
};