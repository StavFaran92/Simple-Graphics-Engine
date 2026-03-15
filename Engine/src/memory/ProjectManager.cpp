#include "memory/ProjectManager.h"

#include "memory/Assets.h"
#include "serialize/Archiver.h"
#include "core/Engine.h"
//#include "TextureSerializer.h"
#include <nlohmann/json.hpp>
#include "runtime/Scene.h"

using json = nlohmann::json;

namespace fs = std::filesystem;

void ProjectManager::loadProject(const std::string& filePath, std::shared_ptr<Context>& context)
{
    Engine::get()->getSubSystem<Assets>()->loadAssetsDatabase();
    Archiver::load();    
}

void ProjectManager::saveProject()
{
    Engine::get()->getSubSystem<Assets>()->saveDirtyAssets();

    // Save Assets
    Engine::get()->getContext()->save();
    Archiver::save();

    SerializedScene serializedScene = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());

    AssetUpdateDescriptor desc;
    desc.makeResourceUpdateDescriptor<SceneCreateDescriptor>()->data.m_serializedScene = serializedScene;
    UUID uid = Engine::get()->getContext()->getActiveSceneAsset().getUID();
    Engine::get()->getSubSystem<Assets>()->updateAsset(uid, desc);
    //Engine::get()->getContext()->getActiveSceneAsset()->updateAsset(desc);
}
