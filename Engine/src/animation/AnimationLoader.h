#pragma once

#include <memory>
#include "memory/Resource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>

#include "core/Core.h"
#include "memory/Assets.h"
#include "memory/Asset.h"

struct AnimationImportSettings : public BaseAssetParameters
{

};

class Animation;

class EngineAPI AnimationLoader
{
public:
	AnimationLoader();

	Resource<Animation> load(AssetInfo& aInfo);

	bool copyFileToResourceFolder(const std::string& fileLocation, AssetInfo&);


private:
	Assimp::Importer m_importer;
};