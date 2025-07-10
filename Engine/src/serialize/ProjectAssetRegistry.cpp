#include "serialize/ProjectAssetRegistry.h"

#include "Logger.h"



#include <filesystem>
#include <fstream>

#include "Assets.h"

using json = nlohmann::json;

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetInfo& asset)
{
    j = nlohmann::json{
        {"uuid", asset.uuid}, // Assuming UUID has a valid to_json
        {"origFilePath", asset.origFilePath},
        {"filePath", asset.filePath},
        {"type", asset.aType}, // Assuming AssetType supports JSON conversion
        {"isValid", asset.isValid},
        {"attributes", asset.attributes},
        {"name", asset.name},
        {"isTransient", asset.isTransient},
    };
}

// Deserialization (from JSON)
void from_json(const nlohmann::json& j, AssetInfo& asset)
{
    j.at("uuid").get_to(asset.uuid); // Assuming UUID has a valid from_json
    j.at("origFilePath").get_to(asset.origFilePath);
    j.at("filePath").get_to(asset.filePath);
    j.at("type").get_to(asset.aType); // Assuming AssetType supports JSON conversion
    j.at("isValid").get_to(asset.isValid);
    j.at("attributes").get_to(asset.attributes);
    j.at("name").get_to(asset.name);
    j.at("isTransient").get_to(asset.isTransient);
}

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
    par->m_assetRegistry["animations"] = nlohmann::json::array();
    par->m_assetRegistry["shaders"] = nlohmann::json::array();
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

std::string getAssetTypeAsStr(AssetType aType)
{
    if (aType == AssetType::MESH) return "meshes";
    if (aType == AssetType::TEXTURE) return "textures";
    if (aType == AssetType::SHADER) return "shaders";
    if (aType == AssetType::ANIMATION) return "animations";
    return "N/A";
}

void ProjectAssetRegistry::addAssetRegistry(AssetInfo asset)
{
    std::string assetTypeName = getAssetTypeAsStr(asset.aType);
    json j;
    to_json(j, asset);
    m_assetRegistry[assetTypeName].push_back(j);
    sync();
}

void ProjectAssetRegistry::addAssociation(std::string name, UUID uuid)
{
    m_assetRegistry["association"].push_back({ name, uuid });
    sync();
}

std::vector<AssetInfo> ProjectAssetRegistry::getAllAssetsOfType(AssetType aType) const
{
    std::string aTypeStr = getAssetTypeAsStr(aType);
    if (!m_assetRegistry.contains(aTypeStr))
    {
        return {};
    }
    return m_assetRegistry[aTypeStr].get<const std::vector<AssetInfo>>();
}

//std::vector<UUID> ProjectAssetRegistry::getMeshList() const
//{
//    if (!m_assetRegistry.contains("meshes"))
//    {
//        return {};
//    }
//    return m_assetRegistry["meshes"].get<const std::vector<UUID>>();
//}
//
//std::vector<UUID> ProjectAssetRegistry::getTextureList() const
//{
//    if (!m_assetRegistry.contains("textures"))
//    {
//        return {};
//    }
//	return m_assetRegistry["textures"].get<std::vector<ProjectAssetRegistry::TextureAsset>>();
//}
//
//std::vector<UUID> ProjectAssetRegistry::getAnimationList() const
//{
//    if (!m_assetRegistry.contains("animations"))
//    {
//        return {};
//    }
//    return m_assetRegistry["animations"].get<std::vector<std::string>>();
//}

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
