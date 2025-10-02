#pragma once

#include <memory>
#include "memory/ResourceWrapper.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>

#include "core/Core.h"
#include "memory/Assets.h"
#include "memory/Asset.h"

struct AnimationImportSettings : public AssetCreateDescriptor
{

};

class Animation;

class EngineAPI AnimationLoader
{
public:
	AnimationLoader();

	ResourceWrapper<Animation> load(const AssetInfo& aInfo);

	bool copyFileToResourceFolder(const std::string& fileLocation, const AssetInfo&);


private:
	Assimp::Importer m_importer;
};