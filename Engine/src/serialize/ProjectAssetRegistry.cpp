#include "serialize/ProjectAssetRegistry.h"

#include "core/Logger.h"



#include <filesystem>
#include <fstream>

#include "memory/Assets.h"

using json = nlohmann::json;

// Serialization (to JSON)
void to_json(nlohmann::json& j, const AssetInfo& asset)
{
	j = nlohmann::json{
		{"uuid", asset.uuid}, // Assuming UUID has a valid to_json
		{"origFilePath", asset.origFilePath},
		{"relativefilePath", asset.relativefilePath},
		{"type", asset.aType}, // Assuming AssetType supports JSON conversion
		{"isValid", asset.isValid},
		{"attributes", asset.attributes},
		{"importSettings", asset.importSettings},
		{"name", asset.name},
		{"isEngineOwned", asset.isEngineOwned},
		{"filename", asset.fileName},
	};
}

// Deserialization (from JSON)
void from_json(const nlohmann::json& j, AssetInfo& asset)
{
	j.at("uuid").get_to(asset.uuid); // Assuming UUID has a valid from_json
	j.at("origFilePath").get_to(asset.origFilePath);
	j.at("relativefilePath").get_to(asset.relativefilePath);
	j.at("type").get_to(asset.aType); // Assuming AssetType supports JSON conversion
	j.at("isValid").get_to(asset.isValid);
	j.at("attributes").get_to(asset.attributes);
	j.at("importSettings").get_to(asset.importSettings);
	j.at("name").get_to(asset.name);
	j.at("isEngineOwned").get_to(asset.isEngineOwned);
	j.at("filename").get_to(asset.fileName);

	asset.establishFilepath();
}

ProjectAssetRegistry::ProjectAssetRegistry(const std::string& filename)
	: m_filename(filename)
{

}

std::shared_ptr<ProjectAssetRegistry> ProjectAssetRegistry::create(const std::string& dirPath)
{
	auto filename = dirPath + "/ProjectAssetRegistry.json";

	std::shared_ptr<ProjectAssetRegistry> par = std::make_shared<ProjectAssetRegistry>(filename);

	for (auto&[_, name] : g_assetTypeToStr)
	{
		par->m_assetRegistry[name] = nlohmann::json::array();
	}

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

	UUID::setCounter(resourceFile["counter"]);

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

void ProjectAssetRegistry::addAssetRegistry(const AssetInfo& asset)
{
	std::string assetTypeName = getAssetTypeAsStr(asset.aType);
	json j;
	to_json(j, asset);
	m_assetRegistry[assetTypeName].push_back(j);
	sync();
}

void ProjectAssetRegistry::updateAssetRegistry(const AssetInfo& asset)
{
	std::string assetTypeName = getAssetTypeAsStr(asset.aType);
	for (auto& aReg : m_assetRegistry[assetTypeName])
	{
		std::string uuid = aReg.at("uuid").get<std::string>();
		if (uuid == asset.uuid.str())
		{
			to_json(aReg, asset);
			sync();
			return;
		}
	}

	addAssetRegistry(asset);
}

void ProjectAssetRegistry::removeAssetRegistry(const AssetInfo& asset)
{
	std::string assetTypeName = getAssetTypeAsStr(asset.aType);
	for (int i=0; i< m_assetRegistry[assetTypeName].size(); i++)
	{
		auto aReg = m_assetRegistry[assetTypeName][i];
		UUID uuid = aReg.at("uuid").get<UUID>();
		if (uuid == asset.uuid)
		{
			m_assetRegistry[assetTypeName].erase(i);
			sync();
			return;
		}
	}
}

//void ProjectAssetRegistry::addAssociation(std::string name, UUID uuid)
//{
//	m_assetRegistry["association"][name] = uuid;
//	sync();
//}
//
//void ProjectAssetRegistry::removeAssociation(std::string name)
//{
//	m_assetRegistry["association"].erase(name);
//	sync();
//}

std::vector<AssetInfo> ProjectAssetRegistry::getAllAssetsOfType(AssetType aType) const
{
	std::string aTypeStr = getAssetTypeAsStr(aType);
	if (!m_assetRegistry.contains(aTypeStr))
	{
		return {};
	}
	return m_assetRegistry[aTypeStr].get<const std::vector<AssetInfo>>();
}

std::vector<AssetInfo> ProjectAssetRegistry::getAllAssets() const
{
	std::vector<AssetInfo> result;

	for (auto& [key, value] : m_assetRegistry.items())
	{
		// skip keys that don't map to AssetInfo
		if (key == "association") // TODO fix!
			continue;

		// try to parse arrays of AssetInfo
		if (value.is_array())
		{
			try {
				auto assetsVec = value.get<std::vector<AssetInfo>>();
				result.insert(result.end(), assetsVec.begin(), assetsVec.end());
			}
			catch (const nlohmann::json::exception& e) {
				// not convertible to AssetInfo, just skip
				continue;
			}
		}
	}

	return result;
}

std::unordered_map<std::string, UUID> ProjectAssetRegistry::getPaths() const
{
	std::unordered_map<std::string, UUID> paths;

	if (m_assetRegistry.contains("pathsToUUIDRefs"))
	{
		const auto& assoc = m_assetRegistry["pathsToUUIDRefs"];

		if (assoc.is_object())
		{
			for (auto it = assoc.begin(); it != assoc.end(); ++it)
			{
				const std::string key = it.key();
				const UUID uuid(it.value().get<uint64_t>());
				paths[key] = uuid;
			}
		}
		else
		{
			logWarning("Expected 'association' to be an object, but got different type.");
		}
	}
	return paths;
}

std::unordered_map<std::string, UUID> ProjectAssetRegistry::getNames() const
{
	std::unordered_map<std::string, UUID> names;

	if (m_assetRegistry.contains("namesToUUIDRefs"))
	{
		const auto& assoc = m_assetRegistry["namesToUUIDRefs"];

		if (assoc.is_object())
		{
			for (auto it = assoc.begin(); it != assoc.end(); ++it)
			{
				const std::string key = it.key();
				const UUID uuid(it.value().get<uint64_t>());
				names[key] = uuid;
			}
		}
		else
		{
			logWarning("Expected 'association' to be an object, but got different type.");
		}
	}
	return names;
}

void ProjectAssetRegistry::syncPaths(const std::unordered_map<std::string, UUID>& pathsReferences)
{
	m_assetRegistry["pathsToUUIDRefs"] = pathsReferences;
	sync();
}

void ProjectAssetRegistry::syncNames(const std::unordered_map<std::string, UUID>& namesReferences)
{
	m_assetRegistry["namesToUUIDRefs"] = namesReferences;
	sync();
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

	m_assetRegistry["counter"] = UUID::getCounter();

	// Write JSON data to file
	outputFile << std::setw(4) << m_assetRegistry << std::endl;
	outputFile.close();
}
