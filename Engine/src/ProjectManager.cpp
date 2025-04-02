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
    
    // Check if the file exists
    if (!fs::exists(filePath)) 
    {
        logError("File not found: " + filePath);
        return;
    }

    // Open file
    std::ifstream projectFile(filePath);
    if (!projectFile.is_open())
    {
        logError("Failed to open file: " + filePath);
        return ;
    }

    // Get file directory
    fs::path projectDir = fs::path(filePath).parent_path();

    // Parse JSON
    json resourceFileJSON;
    try 
    {
        resourceFileJSON = json::parse(projectFile);
    }
    catch (const std::exception& e) 
    {
        logError("Failed to parse JSON: " + std::string(e.what()));
        return ;
    }

    auto par = context->getProjectAssetRegistry();

    std::vector<std::string> meshNameList = par->getMeshList();
    auto textureAssetList = par->getTextureList();
    std::vector<std::string> animationNameList = par->getAnimationList();

    // Create meshes
    for (const auto& meshUID : meshNameList) 
    {
        // TODO fix
        ModelImporter::ModelInfo mInfo;
        std::string binFilePath = (projectDir / meshUID).string() + ".dae";
        MeshCollection* meshPtr = new MeshCollection();
        Resource<MeshCollection> mesh(meshUID);
        mInfo.mesh = mesh;
        Engine::get()->getMemoryPool<MeshCollection>()->add(meshUID, meshPtr);
        Engine::get()->getResourceManager()->incRef(meshUID);
        Engine::get()->getSubSystem<ModelImporter>()->load(binFilePath, mInfo);
    }

    // Create textures
    for (const auto& textureAsset : textureAssetList)
    {
        UUID uid = textureAsset.uuid;
        // Open bin file
        fs::path imageFilePath = (projectDir / uid).string() + "." + textureAsset.ext;
        Texture::loadTexture2D(uid, imageFilePath.string());
    }

    // Create animations
    for (const auto& animUID : animationNameList)
    {
        std::string binFilePath = (projectDir / animUID).string() + ".dae";
        Animation* animPtr = new Animation();
        Resource<Animation> anim(animUID);
        Engine::get()->getMemoryPool<Animation>()->add(animUID, animPtr);
        Engine::get()->getResourceManager()->incRef(animUID);
        Engine::get()->getSubSystem<AnimationLoader>()->load(binFilePath, anim);
    }

    Archiver::load();
}

void ProjectManager::saveProject()
{
    Engine::get()->getContext()->save();
    Archiver::save();
}
