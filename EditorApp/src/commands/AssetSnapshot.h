#pragma once

#include "memory/UUID.h"

#include <string>

// One asset's state, as cereal JSON from Asset::saveToJson
struct AssetSnapshot
{
	UUID asset;
	std::string data;
};

AssetSnapshot takeAssetSnapshot(UUID asset);

// Puts the data back and marks the asset dirty, so its resource picks it up next frame
void applyAssetSnapshot(const AssetSnapshot& snapshot);
