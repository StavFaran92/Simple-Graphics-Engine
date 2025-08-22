#pragma once

#include "Assets.h"

class AssetFactory
{
public:
	using LoadFn = std::function<void(AssetInfo)>;

	static bool loadAsset(AssetInfo aInfo);

	static void registerLoadFunc(AssetType aType, const LoadFn& fn);

private:
	static std::map<AssetType, LoadFn> loadFunctionsRegistry;
};