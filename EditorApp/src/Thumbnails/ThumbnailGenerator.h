#pragma once

#include "sge.h"

#include <memory>
#include <unordered_map>

// Matches the card's thumbnail area, so letterboxing and transparency blend into the card
inline const glm::vec3 g_thumbnailBackground(26.f / 255.f);

// Thumbnails are stored top-down (row 0 is the top of the image) so they draw with ImGui's default UVs
class IThumbnailGenerator
{
public:
	virtual ~IThumbnailGenerator() = default;

	// Draws the asset's thumbnail into target, overwriting it.
	// Returns false if no thumbnail can be made, in which case the type icon is used.
	virtual bool generate(const AssetRecord& aInfo, const TextureResourceRef& target) = 0;
};

class ThumbnailGeneratorRegistry
{
public:
	void registerGenerator(AssetType aType, std::unique_ptr<IThumbnailGenerator> generator)
	{
		m_generators[aType] = std::move(generator);
	}

	// nullptr if no generator is registered for the type
	IThumbnailGenerator* getGenerator(AssetType aType) const
	{
		auto iter = m_generators.find(aType);
		return iter != m_generators.end() ? iter->second.get() : nullptr;
	}

private:
	std::unordered_map<AssetType, std::unique_ptr<IThumbnailGenerator>> m_generators;
};
