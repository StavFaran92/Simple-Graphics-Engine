#pragma once

#include "memory/UUID.h"

#include <string>

// One asset's state as cereal JSON: its registry metadata and the data it holds
struct AssetSnapshot
{
	UUID asset;
	std::string metadata;
	std::string data;
};

AssetSnapshot takeAssetSnapshot(UUID asset);

// Puts the data back and marks the asset dirty, so its resource picks it up next frame
void applyAssetSnapshot(const AssetSnapshot& snapshot);
