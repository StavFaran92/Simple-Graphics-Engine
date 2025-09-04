#pragma once

#include "Assets.h"

class AssetFactory
{
public:
	using LoadFn = std::function<Resource<Asset>(AssetInfo&)>;

	static bool loadAsset(AssetInfo& aInfo);

	static void registerLoadFunc(AssetType aType, const LoadFn& fn);

private:

	// Magic static
	static std::map<AssetType, LoadFn>& getLoadFunctionRegistry() {
		static std::map<AssetType, LoadFn> loadFunctionsRegistry;
		return loadFunctionsRegistry;
	}

private:
	
};