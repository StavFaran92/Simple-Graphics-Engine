#include "RegisterManagers.h"

#include "memory/AssetFactory.h"
#include "texture/TextureManager.h"
#include "render/ShaderManager.h"
#include "render/MaterialManager.h"
#include "geometry/MeshGroupManager.h"
#include "animation/AnimationManager.h"
#include "scripts/LuaScriptManager.h"
#include "runtime/SceneManager.h"
#include "runtime/PrefabManager.h"

void registerAllManagers()
{
	AssetFactory::registerManager(AssetType::TEXTURE,    std::make_shared<TextureTypeManager>());
	AssetFactory::registerManager(AssetType::SHADER,     std::make_shared<ShaderTypeManager>());
	AssetFactory::registerManager(AssetType::MATERIAL,   std::make_shared<MaterialTypeManager>());
	AssetFactory::registerManager(AssetType::MESH,       std::make_shared<MeshGroupTypeManager>());
	AssetFactory::registerManager(AssetType::ANIMATION,  std::make_shared<AnimationTypeManager>());
	AssetFactory::registerManager(AssetType::LUA_SCRIPT, std::make_shared<LuaScriptTypeManager>());
	AssetFactory::registerManager(AssetType::SCENE,      std::make_shared<SceneTypeManager>());
	AssetFactory::registerManager(AssetType::PREFAB,     std::make_shared<PrefabTypeManager>());
}
