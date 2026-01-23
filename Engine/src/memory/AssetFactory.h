#pragma once

#include "Assets.h"

class AssetManager;

class AssetFactory
{
public:
	using LoadFn = std::function<AssetWrapper<Resource>(AssetInfo&)>;

	// Register a manager instance for a specific asset type.
	static void registerManager(AssetType aType, std::shared_ptr<AssetManager> manager);

	// Retrieve the manager (non-owning pointer).
	static AssetManager* getManager(AssetType aType);

	static bool loadAsset(AssetInfo& aInfo);

	static void registerLoadFunc(AssetType aType, const LoadFn& fn);


private:

	// Magic static
	static std::map<AssetType, LoadFn>& getLoadFunctionRegistry() {
		static std::map<AssetType, LoadFn> loadFunctionsRegistry;
		return loadFunctionsRegistry;
	}

private:
	static std::map<AssetType, std::shared_ptr<AssetManager>>& getManagerRegistry();
};