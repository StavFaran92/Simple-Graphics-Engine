#pragma once

#include <memory>
#include "memory/Resource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>

#include "core/Core.h"
#include "memory/Assets.h"

class Animation;

struct AnimationImportSettings
{
	std::string name;
};

class EngineAPI AnimationLoader
{
public:
	

	AnimationLoader();

	Resource<Animation> import(const std::string& path, AnimationImportSettings settings = {});

	Resource<Animation> load(AssetInfo aInfo);

private:
	Assimp::Importer m_importer;
};