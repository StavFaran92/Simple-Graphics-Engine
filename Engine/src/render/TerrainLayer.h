#pragma once

#include <string>
#include "memory/AssetAliases.h"

#include "serialize/CerealHelpers.h"

struct TerrainLayer
{
	std::string name = "Layer";

	TextureResourceRef texturePack0;
	TextureResourceRef texturePack1;
	TextureResourceRef mask;
	glm::vec2 uv;
};

struct TerrainLayerAsset
{
	std::string name = "Layer";

	TextureAssetRef albedoTexture;
	TextureAssetRef normalTexture;
	TextureAssetRef metallicTexture;
	TextureAssetRef roughnessTexture;
	TextureAssetRef aoTexture;
	TextureAssetRef mask;
	glm::vec2 uv;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(albedoTexture);
		SERIALIZED_MEMBER(normalTexture);
		SERIALIZED_MEMBER(metallicTexture);
		SERIALIZED_MEMBER(roughnessTexture);
		SERIALIZED_MEMBER(aoTexture);
		SERIALIZED_MEMBER(mask);
		SERIALIZED_MEMBER(uv);
	}
};