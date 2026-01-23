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

#include "systems/SubSystem.h"

struct AnimationImportSettings : public AssetCreateDescriptor
{

};

class Animation;

class EngineAPI AnimationLoader : public SubSystem
{
public:
	AnimationLoader();

	ResourceWrapper<Animation> load(const AssetRecord& aInfo);

	bool copyFileToResourceFolder(const std::string& fileLocation, const AssetRecord&);


private:
	Assimp::Importer m_importer;
};