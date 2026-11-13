#include "TerrainCommands.h"
#include "ComponentSnapshot.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
	bool isEmpty(const HeightmapRect& r)
	{
		return r.width <= 0 || r.height <= 0;
	}
}

HeightmapSnapshot takeHeightmapSnapshot(Entity e, HeightmapRect rect)
{
	Terrain& terrain = e.getComponent<Terrain>();
	TextureResourceRef texture = terrain.getHeightmap();

	HeightmapSnapshot snapshot;
	snapshot.entity = e.handlerID();
	snapshot.heightmap = terrain.getHeightmapAsset().getUID();
	snapshot.rect = rect;
	snapshot.heights.resize(size_t(std::max(rect.width, 0)) * std::max(rect.height, 0));

	// The CPU copy is current: every brush dab ends with syncHeightmap()
	const float* src = texture->getData().data.getFloatData();
	const int mapWidth = texture->getWidth();
	for (int row = 0; row < rect.height; row++)
	{
		const float* from = src + size_t(rect.y + row) * mapWidth + rect.x;
		std::copy(from, from + rect.width, snapshot.heights.begin() + size_t(row) * rect.width);
	}

	return snapshot;
}

HeightmapSnapshot cropSnapshot(const HeightmapSnapshot& snapshot, HeightmapRect rect)
{
	HeightmapSnapshot cropped;
	cropped.entity = snapshot.entity;
	cropped.heightmap = snapshot.heightmap;
	cropped.rect = rect;
	cropped.heights.resize(size_t(std::max(rect.width, 0)) * std::max(rect.height, 0));

	// Offsets of rect inside the snapshot's own rect
	const int dx = rect.x - snapshot.rect.x;
	const int dy = rect.y - snapshot.rect.y;
	for (int row = 0; row < rect.height; row++)
	{
		auto from = snapshot.heights.begin() + size_t(dy + row) * snapshot.rect.width + dx;
		std::copy(from, from + rect.width, cropped.heights.begin() + size_t(row) * rect.width);
	}

	return cropped;
}

void applyHeightmapSnapshot(const HeightmapSnapshot& snapshot)
{
	Entity e = entityFromId(snapshot.entity);
	Terrain* terrain = e.valid() ? e.tryGetComponent<Terrain>() : nullptr;
	if (!terrain)
		throw std::runtime_error("Terrain entity " + std::to_string(snapshot.entity) + " no longer exists");

	if (terrain->getHeightmapAsset().getUID() != snapshot.heightmap)
		throw std::runtime_error("the terrain was resized since this edit");

	if (isEmpty(snapshot.rect))
		return;

	const HeightmapRect& r = snapshot.rect;
	terrain->getHeightmap()->setData(r.x, r.y, r.width, r.height, snapshot.heights.data());

	// Brings the CPU copy in line and marks the asset for saving
	terrain->syncHeightmap();
}

HeightmapRect dabRect(glm::vec2 centerPixel, float radiusPixels, const TextureResourceRef& texture)
{
	const int x0 = std::max(0, int(std::floor(centerPixel.x - radiusPixels)));
	const int y0 = std::max(0, int(std::floor(centerPixel.y - radiusPixels)));
	const int x1 = std::min(texture->getWidth() - 1, int(std::ceil(centerPixel.x + radiusPixels)));
	const int y1 = std::min(texture->getHeight() - 1, int(std::ceil(centerPixel.y + radiusPixels)));

	if (x1 < x0 || y1 < y0)
		return {};

	return { x0, y0, x1 - x0 + 1, y1 - y0 + 1 };
}

HeightmapRect unionRect(const HeightmapRect& a, const HeightmapRect& b)
{
	if (isEmpty(a))
		return b;
	if (isEmpty(b))
		return a;

	const int x0 = std::min(a.x, b.x);
	const int y0 = std::min(a.y, b.y);
	const int x1 = std::max(a.x + a.width, b.x + b.width);
	const int y1 = std::max(a.y + a.height, b.y + b.height);
	return { x0, y0, x1 - x0, y1 - y0 };
}

EditHeightmapCmd::EditHeightmapCmd(HeightmapSnapshot before, HeightmapSnapshot after, std::string label)
	: m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label))
{
}

bool EditHeightmapCmd::execute(std::string&)
{
	applyHeightmapSnapshot(m_after);
	return true;
}

void EditHeightmapCmd::undo()
{
	applyHeightmapSnapshot(m_before);
}
