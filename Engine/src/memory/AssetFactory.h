#pragma once

#include "Assets.h"
#include "memory/ResourcePipeline.h"

class AssetFactory
{
public:
	using LoadFn = std::function<AssetHandle<Asset>(AssetRecord&)>;

	// Register a resource type manager for a specific asset type.
	static void registerManager(AssetType aType, std::shared_ptr<ResourceTypeManager> manager);

	// Retrieve the manager (non-owning pointer).
	static ResourceTypeManager* getManager(AssetType aType);

	static bool loadAsset(AssetRecord& aInfo);

	static void registerLoadFunc(AssetType aType, const LoadFn& fn);


private:

	// Magic static
	static std::map<AssetType, LoadFn>& getLoadFunctionRegistry() {
		static std::map<AssetType, LoadFn> loadFunctionsRegistry;
		return loadFunctionsRegistry;
	}

private:
	static std::map<AssetType, std::shared_ptr<ResourceTypeManager>>& getManagerRegistry();
};
