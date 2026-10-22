#pragma once

#include "memory/UUID.h"

#include <nlohmann/json.hpp>

// One asset's state, as its own Asset::serialize writes it
struct AssetSnapshot
{
	UUID asset;
	nlohmann::json data;
};

AssetSnapshot takeAssetSnapshot(UUID asset);

// Puts the data back and marks the asset dirty, so its resource picks it up next frame
void applyAssetSnapshot(const AssetSnapshot& snapshot);
