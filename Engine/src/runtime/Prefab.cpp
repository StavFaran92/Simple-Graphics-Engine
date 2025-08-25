#include "Prefab.h"

#include <filesystem>

static AssetFnRegister<AssetType::PREFAB> assetRegister(Prefab::load);

Resource<Prefab> Prefab::import(const std::string& fileLocation, const PrefabImportSettings& settings)
{
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logWarning("Invalid file location specified.");
		return Resource<Prefab>::empty;
	}

	std::filesystem::path file(fileLocation);
	if (file.extension() != ".asset")
	{
		logWarning("Invalid file type specified, prefab should have .asset extension.");
		return Resource<Prefab>::empty;
	}

	AssetInfo aInfo;
	aInfo.uuid = "test";
	aInfo.origFilePath = fileLocation;
	aInfo.aType = AssetType::PREFAB;
	aInfo.name = file.filename().stem().string();
	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);

	return load(aInfo);
}

Resource<Prefab> Prefab::load(AssetInfo aInfo)
{
    return Resource<Prefab>();
}
