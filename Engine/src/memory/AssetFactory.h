#pragma once

#include "Assets.h"

class AssetManager;

class AssetFactory
{
public:
	// Register a manager instance for a specific asset type.
	static void registerManager(AssetType aType, std::shared_ptr<AssetManager> manager);

	// Retrieve the manager (non-owning pointer).
	static AssetManager* getManager(AssetType aType);

private:
	static std::map<AssetType, std::shared_ptr<AssetManager>>& getManagerRegistry();
};