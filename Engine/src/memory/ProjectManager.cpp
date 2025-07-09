#include "ProjectManager.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "Logger.h"
#include "Mesh.h"
#include "ProjectAssetRegistry.h"
#include "MeshSerializer.h"
#include "Texture.h"
#include "Assets.h"
#include "Archiver.h"
#include "ModelImporter.h"
#include "AnimationLoader.h"
#include "MeshCollection.h"
//#include "TextureSerializer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace fs = std::filesystem;

void ProjectManager::loadProject(const std::string& filePath, std::shared_ptr<Context>& context)
{
    Engine::get()->getSubSystem<Assets>()->load();
    Archiver::load();    
}

void ProjectManager::saveProject()
{
    Engine::get()->getContext()->save();
    Archiver::save();
}
