#pragma once

#include <memory>
#include "memory/Resource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>

#include "core/Core.h"
#include "memory/Assets.h"
#include "memory/AssetLoader.h"

class Animation;

struct AnimationImportSettings : BaseAssetParameters
{
	std::string name;
};

class EngineAPI AnimationLoader : public AssetLoader
{
public:
	

	AnimationLoader();

	//Resource<Asset> import(const std::string& fileLocation, const BaseAssetParameters& params);

	Resource<Asset> load(AssetInfo& aInfo) override;

	std::string copyFileToResourceFolder(const std::string& fileLocation, AssetInfo&) override;

private:
	Assimp::Importer m_importer;

	// Inherited via AssetLoader
	void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo) override;
};