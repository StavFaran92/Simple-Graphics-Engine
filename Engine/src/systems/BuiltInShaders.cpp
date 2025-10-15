#include "systems/BuiltInMeshes.h"

#include "geometry/Quad.h"
#include "geometry/Box.h"
#include "geometry/Sphere.h"
#include "geometry/Grid.h"

#include "memory/Assets.h"
#include "geometry/ModelImporter.h"
#include "core/Factory.h"
#include "BuiltInShaders.h"

void addAsAsset(const ResourceWrapper<Shader>& shader)
{
	AssetCreateDescriptor aInfo;
	aInfo.customUUID = shader.getUID();
	aInfo.aType = AssetType::SHADER;
	aInfo.name = shader.getUID();
	aInfo.isEngineOwned = true;
	Engine::get()->getSubSystem<Assets>()->createAsset(shader, aInfo);
}

BuiltInShaders::BuiltInShaders()
{
}
