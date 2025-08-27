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
	
};

class EngineAPI AnimationLoader : public AssetLoader
{
public:
	AnimationLoader();

	Resource<Asset> load(AssetInfo& aInfo) override;


private:
	Assimp::Importer m_importer;

private:
	bool copyFileToResourceFolder(const std::string& fileLocation, AssetInfo&) override;
	void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo) override;
};