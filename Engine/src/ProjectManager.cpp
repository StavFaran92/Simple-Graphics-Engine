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
    std::vector<std::string> textureNameList = par->getTextureList();
    std::vector<std::string> animationNameList = par->getAnimationList();

    // Create meshes
    for (const auto& meshUID : meshNameList) 
    {
        std::string binFilePath = (projectDir / meshUID).string() + ".dae";
        Mesh* meshPtr = new Mesh();
        Resource<Mesh> mesh(meshUID);
        Engine::get()->getMemoryPool<Mesh>()->add(meshUID, meshPtr);
        Engine::get()->getResourceManager()->incRef(meshUID);
        Engine::get()->getSubSystem<ModelImporter>()->load(binFilePath, mesh);        
    }

    // Create textures
    for (const auto& textureUID : textureNameList) 
    {
        // Open bin file
        fs::path imageFilePath = (projectDir / textureUID).string() + ".png";
        Engine::get()->getSubSystem<Assets>()->loadTexture2D(textureUID, imageFilePath.string());
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
