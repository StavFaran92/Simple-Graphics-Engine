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

    // Create meshes
    for (const auto& meshUID : meshNameList) 
    {
        // Open bin file
        fs::path binFilePath = (projectDir / meshUID).string() + ".bin";
        std::ifstream binFile(binFilePath, std::ios::binary);
        if (!binFile.is_open()) 
        {
            logError("Failed to open bin file for mesh: " + meshUID + ".bin");
            continue; // Skip this mesh
        }
        MeshData meshData;
        MeshSerializer::readDataFromBinaryFile(binFilePath.string(), meshData);

        // Create mesh
        Mesh* mesh = new Mesh();
        mesh->build(meshData);
        Engine::get()->getMemoryPool<Mesh>()->add(meshUID, mesh);
        Engine::get()->getResourceManager()->incRef(meshUID);
    }

    // Create textures
    for (const auto& textureUID : textureNameList) 
    {
        // Open bin file
        fs::path imageFilePath = (projectDir / textureUID).string() + ".png";
        Texture::TextureData textureData = Assets::extractTextureDataFromFile(imageFilePath.string());

        // Create texture
        Texture* texture = new Texture();
        texture->build(textureData);
        Engine::get()->getMemoryPool<Texture>()->add(textureUID, texture);
        Engine::get()->getResourceManager()->incRef(textureUID);
    }

    context->deserialize();
}

void ProjectManager::saveProject()
{
    Engine::get()->getContext()->serialize();
}
