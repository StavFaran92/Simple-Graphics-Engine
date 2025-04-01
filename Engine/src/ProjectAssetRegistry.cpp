#include "ProjectAssetRegistry.h"

#include "Logger.h"



#include <filesystem>
#include <fstream>



using json = nlohmann::json;

ProjectAssetRegistry::ProjectAssetRegistry(const std::string& filename)
    : m_filename(filename)
{

}

std::shared_ptr<ProjectAssetRegistry> ProjectAssetRegistry::create(const std::string& dirPath)
{
    auto filename = dirPath + "/ProjectAssetRegistry.json";

    std::shared_ptr<ProjectAssetRegistry> par = std::make_shared<ProjectAssetRegistry>(filename);

    // Create JSON object with empty arrays for meshes and textures
    par->m_assetRegistry["meshes"] = nlohmann::json::array();
    par->m_assetRegistry["textures"] = nlohmann::json::array();
    par->m_assetRegistry["association"] = nlohmann::json::array();

    // Write JSON data to file
    std::ofstream outputFile(filename);
    if (!outputFile.is_open())
    {
        logError("Failed to create asset registry file: " + filename);
        return nullptr;
    }
    outputFile << std::setw(4) << par->m_assetRegistry << std::endl;
    outputFile.close();

    // Log success
    logInfo("Asset registry file created: " + filename);

    return par;
}

std::shared_ptr<ProjectAssetRegistry> ProjectAssetRegistry::parse(const std::string& dirPath)
{
    auto filename = dirPath + "/ProjectAssetRegistry.json";

    std::shared_ptr<ProjectAssetRegistry> par = std::make_shared<ProjectAssetRegistry>(filename);

    std::ifstream outputFile(filename);
    if (!outputFile.is_open())
    {
        logError("Failed to open asset registry file: " + filename);
        return nullptr;
    }

    std::string file_content((std::istreambuf_iterator<char>(outputFile)), std::istreambuf_iterator<char>());

    auto resourceFile = json::parse(file_content, nullptr, false);

    if (resourceFile.is_discarded())
    {
        logError("Failed to load json file");
        return nullptr;
    }

    par->m_assetRegistry = resourceFile;

    return par;
}

std::string ProjectAssetRegistry::getFilepath() const
{
    return m_filename;
}

void ProjectAssetRegistry::reset()
{

	// reset the asset registry file
}

void ProjectAssetRegistry::save()
{
    sync();
}

void ProjectAssetRegistry::addMesh(UUID uuid)
{
    m_assetRegistry["meshes"].push_back(uuid);
    sync();
}

void ProjectAssetRegistry::addTexture(Resource<Texture> texture)
{
    // Create a JSON object with UUID and format fields
    TextureAsset tMetadata = { 
        texture.getUID(), 
        texture.get()->getData().isHDR ? "hdr" : "png"
    };

    // Add the metadata to the textures array
    m_assetRegistry["textures"].push_back(tMetadata);

    // Sync the registry
    sync();
}

void ProjectAssetRegistry::addAnimation(UUID uuid)
{
    m_assetRegistry["animations"].push_back(uuid);
    sync();
}

void ProjectAssetRegistry::addShader(UUID uuid)
{
    m_assetRegistry["shaders"].push_back(uuid);
    sync();
}

std::string getAssetTypeAsStr(AssetType aType)
{
    if (aType == AssetType::MESH) return "meshes";
    if (aType == AssetType::TEXTURE) return "texture";
    if (aType == AssetType::SHADER) return "shaders";
    if (aType == AssetType::ANIMATION) return "animations";
    return "N/A";
}

void ProjectAssetRegistry::addAssetRegistry(UUID uuid, AssetType aType)
{
    std::string assetTypeName = getAssetTypeAsStr(aType);
    m_assetRegistry[assetTypeName].push_back(uuid);
    sync();
}

void ProjectAssetRegistry::addAssociation(std::string name, UUID uuid)
{
    m_assetRegistry["association"].push_back({ name, uuid });
    sync();
}

std::vector<UUID> ProjectAssetRegistry::getMeshList() const
{
    if (!m_assetRegistry.contains("meshes"))
    {
        return {};
    }
    return m_assetRegistry["meshes"].get<const std::vector<UUID>>();
}

std::vector<ProjectAssetRegistry::TextureAsset> ProjectAssetRegistry::getTextureList() const
{
    if (!m_assetRegistry.contains("textures"))
    {
        return {};
    }
	return m_assetRegistry["textures"].get<std::vector<ProjectAssetRegistry::TextureAsset>>();
}

std::vector<UUID> ProjectAssetRegistry::getAnimationList() const
{
    if (!m_assetRegistry.contains("animations"))
    {
        return {};
    }
    return m_assetRegistry["animations"].get<std::vector<std::string>>();
}

std::unordered_map<std::string, UUID> ProjectAssetRegistry::getAssociations() const
{
    std::unordered_map<std::string, UUID> associations;

    // Check if the "association" key exists and is an array
    if (m_assetRegistry.contains("association") && m_assetRegistry["association"].is_array()) {
        for (const auto& item : m_assetRegistry["association"]) {
            // Ensure each item is an array with exactly two elements
            if (item.is_array() && item.size() == 2 && item[0].is_string() && item[1].is_string()) {
                std::string key = item[0];
                UUID value = item[1];
                associations[key] = value;
            }
        }
    }
    return associations;
}

void ProjectAssetRegistry::sync()
{
    // Open file in write mode to clear its contents
    std::ofstream outputFile(m_filename, std::ios::trunc);
    if (!outputFile.is_open()) 
    {
        logError("Failed to open asset registry file: " + m_filename);
        return;
    }

    // Write JSON data to file
    outputFile << std::setw(4) << m_assetRegistry << std::endl;
    outputFile.close();
}
