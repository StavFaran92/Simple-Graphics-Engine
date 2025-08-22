#include "Prefab.h"

static AssetFnRegister<AssetType::PREFAB> assetRegister(Prefab::load);

Resource<Prefab> Prefab::import(const std::string& fileLocation, const PrefabImportSettings& settings)
{
	AssetInfo aInfo;
	aInfo.uuid = "test";
	aInfo.origFilePath = "C:/Users/Stav/Downloads/blueprint_8974271.png";
	aInfo.aType = AssetType::PREFAB;
	aInfo.name = "test";
	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);

	return Resource<Prefab>::empty;
}

Resource<Prefab> Prefab::load(AssetInfo aInfo)
{
    return Resource<Prefab>();
}
