#include "memory/ProjectManager.h"

#include "memory/Assets.h"
#include "serialize/Archiver.h"
#include "core/Engine.h"
//#include "TextureSerializer.h"
#include <nlohmann/json.hpp>
#include "runtime/Scene.h"
#include "core/ProjectSettings.h"

using json = nlohmann::json;

namespace fs = std::filesystem;

void ProjectManager::loadProject(const std::string& filePath, std::shared_ptr<Context>& context)
{
    ProjectSettings::get().load(Engine::get()->getProjectDirectory());
    Engine::get()->getSubSystem<Assets>()->loadAssetsDatabase();
    Archiver::load();

    //UUID uid activeSceneID = Engine::get()->getProjectSettings()->get<int>("active_scene");
    //SceneAssetRef activeSceneAsset(uid);
    //activeSceneAsset.resource()->getID()
    //Engine::get()->getContext()->setActiveScene()
}

void ProjectManager::saveProject()
{
    Engine::get()->getSubSystem<Assets>()->saveDirtyAssets();

    // Save Assets
    ProjectSettings::get().save();
    Engine::get()->getContext()->save();
    Archiver::save();
    
    // save active scene
    //SerializedScene serializedScene = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());
    AssetUpdateDescriptor desc;
    SceneCreateDescriptor sceneDesc;
    SerializedScene serializedScene = Archiver::serializeScene(Engine::get()->getContext()->getActiveScene());
    sceneDesc.data.m_serializedScene = serializedScene;
    //sceneDesc.data.registryStream = Engine::get()->getContext()->getActiveScene()->getRegistry().toStream();
    UUID uid = Engine::get()->getContext()->getActiveSceneAsset().getUID();
    Engine::get()->getSubSystem<Assets>()->updateAsset(uid, desc, &sceneDesc);

    //Engine::get()->getProjectSettings()->sync();
}
