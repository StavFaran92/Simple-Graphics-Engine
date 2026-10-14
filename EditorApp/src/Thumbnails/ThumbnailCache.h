#pragma once

#include "sge.h"
#include "imgui.h"
#include "ThumbnailGenerator.h"

#include <deque>
#include <unordered_map>

// Lazily generated asset thumbnails, keyed by asset UUID.
// get() never generates; it queues the asset and update() generates a few per frame.
// Past the cap, the entries with the oldest lastUsedFrame are evicted, except ones drawn in the last frame.
class ThumbnailCache
{
public:
	static constexpr int ThumbnailSize = 128;
	static constexpr int MaxEntries = 256;
	static constexpr int GenerationsPerFrame = 2;

	static ThumbnailCache& Instance()
	{
		static ThumbnailCache instance;
		return instance;
	}

	ThumbnailGeneratorRegistry& getRegistry() { return m_registry; }

	// The asset's thumbnail, or 0 if there is none (yet); callers fall back to the type icon
	ImTextureID get(const AssetRecord& aInfo);

	// Regenerates the thumbnail into its existing texture, or drops it if the asset was deleted
	void invalidate(UUID uuid);

	// Call once per frame before any window draws
	void update();

	ThumbnailCache(const ThumbnailCache&) = delete;
	ThumbnailCache& operator=(const ThumbnailCache&) = delete;

private:
	ThumbnailCache() = default;

	enum class State
	{
		Pending,
		Ready,
		Failed
	};

	struct Entry
	{
		TextureResourceRef texture;
		State state = State::Pending;
		bool hasImage = false; // texture holds a valid thumbnail (kept while regenerating)
		int lastUsedFrame = 0;
	};

	void evict();
	void generatePending();

	ThumbnailGeneratorRegistry m_registry;

	std::unordered_map<UUID, Entry> m_entries;
	std::deque<UUID> m_pending;
};
