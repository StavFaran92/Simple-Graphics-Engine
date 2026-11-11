#include "AssetSnapshot.h"

#include "sge.h"
#include "memory/Asset.h"
#include "Thumbnails/ThumbnailCache.h"

#include <stdexcept>

namespace
{
	AssetRef<Asset> existingAsset(UUID uuid)
	{
		AssetRef<Asset> asset(uuid);
		if (asset.isEmpty())
			throw std::runtime_error("Asset " + uuid.str() + " no longer exists");
		return asset;
	}
}

AssetSnapshot takeAssetSnapshot(UUID asset)
{
	AssetSnapshot snapshot;
	snapshot.asset = asset;
	AssetRef<Asset> existing = existingAsset(asset);
	snapshot.metadata = existing->metadataToJson();
	snapshot.data = existing->dataToJson();
	return snapshot;
}

void applyAssetSnapshot(const AssetSnapshot& snapshot)
{
	AssetRef<Asset> asset = existingAsset(snapshot.asset);
	asset->metadataFromJson(snapshot.metadata);
	asset->dataFromJson(snapshot.data);
	asset.makeDirty();
	ThumbnailCache::Instance().invalidate(snapshot.asset);
}
