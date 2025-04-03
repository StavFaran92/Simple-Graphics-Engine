#pragma once

#include "Engine.h"
#include "Resource.h"
#include "Animation.h"
#include "Shader.h"
#include "ModelImporter.h"
#include "Core.h"

#include <functional>

struct AssetInfo
{
	UUID uuid;
	std::string origFilePath;
	std::string filePath;
	AssetType aType;
	bool isValid = false;
	std::map<std::string, std::string> attributes;
	std::string name;
	//bool isClient = false
	//timestamp
	//size
	//etc..
};

class EngineAPI Assets
{
public:
	

	Assets();

	Resource<Animation> importAnimation(const std::string& path);
	Resource<Animation> loadAnimation(UUID uid, const std::string& path);
	std::vector<std::string> getAllAnimations() const;


	std::string getAlias(UUID uid) const;

	template<typename T>
	Resource<T> loadAsset(UUID uid, const std::string& path);

	AssetInfo importAsset(AssetInfo assetInfo);

	AssetInfo addAsset(AssetInfo assetInfo);

	std::vector<std::string> getAllAssetsOfType(AssetType aType) const;

private:
	std::map<AssetType, std::unordered_set<UUID>> m_assets;
	std::unordered_map<UUID, Resource<MeshCollection>> m_meshes;
	std::unordered_map<UUID, Resource<Texture>> m_textures;
	std::unordered_map<UUID, Resource<Animation>> m_animations;
	std::unordered_map<UUID, Resource<Shader>> m_shaders;
};