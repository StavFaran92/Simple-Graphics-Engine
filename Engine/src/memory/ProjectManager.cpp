#include "memory/ProjectManager.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "core/Logger.h"
#include "geometry/Mesh.h"
#include "serialize/ProjectAssetRegistry.h"
#include "geometry/MeshSerializer.h"
#include "texture/Texture.h"
#include "memory/Assets.h"
#include "serialize/Archiver.h"
#include "geometry/ModelImporter.h"
#include "animation/AnimationLoader.h"
#include "geometry/MeshCollection.h"
//#include "TextureSerializer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

void ProjectManager::loadProject(const std::string& filePath, std::shared_ptr<Context>& context)
{
    Engine::get()->getSubSystem<Assets>()->loadAssetsDatabase();
    Archiver::load();    
}

void ProjectManager::saveProject()
{
    Engine::get()->getContext()->save();
    Archiver::save();
}
