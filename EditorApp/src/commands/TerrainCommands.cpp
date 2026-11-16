#include "TerrainCommands.h"
#include "CommandStack.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace
{
	bool isEmpty(const TextureRect& r)
	{
		return r.width <= 0 || r.height <= 0;
	}
}

TextureSnapshot takeTextureSnapshot(const TextureAssetRef& texture, TextureRect rect)
{
	TextureResourceRef resource = texture.resource();

	TextureSnapshot snapshot;
	snapshot.texture = texture.getUID();
	snapshot.rect = rect;

	// A mask that was never painted may have no CPU copy yet; after that every dab downloads it
	const size_t pixelCount = size_t(resource->getWidth()) * resource->getHeight();
	if (resource->getData().data.size() < pixelCount)
		resource->download();

	const ImageBuffer& src = resource->getData().data;
	snapshot.bytesPerPixel = pixelCount ? int(src.size() / pixelCount) : 0;

	const size_t rowBytes = size_t(std::max(rect.width, 0)) * snapshot.bytesPerPixel;
	snapshot.pixels.resize(rowBytes * std::max(rect.height, 0));

	const size_t mapRowBytes = size_t(resource->getWidth()) * snapshot.bytesPerPixel;
	for (int row = 0; row < rect.height; row++)
	{
		const uint8_t* from = src.data() + size_t(rect.y + row) * mapRowBytes + size_t(rect.x) * snapshot.bytesPerPixel;
		std::memcpy(snapshot.pixels.data() + size_t(row) * rowBytes, from, rowBytes);
	}

	return snapshot;
}

TextureSnapshot cropSnapshot(const TextureSnapshot& snapshot, TextureRect rect)
{
	TextureSnapshot cropped;
	cropped.texture = snapshot.texture;
	cropped.rect = rect;
	cropped.bytesPerPixel = snapshot.bytesPerPixel;

	const size_t rowBytes = size_t(std::max(rect.width, 0)) * snapshot.bytesPerPixel;
	cropped.pixels.resize(rowBytes * std::max(rect.height, 0));

	// Offsets of rect inside the snapshot's own rect
	const int dx = rect.x - snapshot.rect.x;
	const int dy = rect.y - snapshot.rect.y;
	const size_t srcRowBytes = size_t(snapshot.rect.width) * snapshot.bytesPerPixel;
	for (int row = 0; row < rect.height; row++)
	{
		const uint8_t* from = snapshot.pixels.data() + size_t(dy + row) * srcRowBytes + size_t(dx) * snapshot.bytesPerPixel;
		std::memcpy(cropped.pixels.data() + size_t(row) * rowBytes, from, rowBytes);
	}

	return cropped;
}

void applyTextureSnapshot(const TextureSnapshot& snapshot)
{
	TextureAssetRef asset(snapshot.texture);
	if (asset.isEmpty())
		throw std::runtime_error("the texture no longer exists (the terrain was resized since this edit)");

	if (isEmpty(snapshot.rect))
		return;

	const TextureRect& r = snapshot.rect;
	TextureResourceRef resource = asset.resource();
	resource->setData(r.x, r.y, r.width, r.height, snapshot.pixels.data());

	// Brings the CPU copy in line and marks the asset for saving
	resource->download();
	asset.makeDirty();
}

TextureRect dabRect(glm::vec2 centerPixel, float radiusPixels, const TextureResourceRef& texture)
{
	const int x0 = std::max(0, int(std::floor(centerPixel.x - radiusPixels)));
	const int y0 = std::max(0, int(std::floor(centerPixel.y - radiusPixels)));
	const int x1 = std::min(texture->getWidth() - 1, int(std::ceil(centerPixel.x + radiusPixels)));
	const int y1 = std::min(texture->getHeight() - 1, int(std::ceil(centerPixel.y + radiusPixels)));

	if (x1 < x0 || y1 < y0)
		return {};

	return { x0, y0, x1 - x0 + 1, y1 - y0 + 1 };
}

TextureRect unionRect(const TextureRect& a, const TextureRect& b)
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

EditTextureCmd::EditTextureCmd(TextureSnapshot before, TextureSnapshot after, std::string label)
	: m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label))
{
}

bool EditTextureCmd::execute(std::string&)
{
	applyTextureSnapshot(m_after);
	return true;
}

void EditTextureCmd::undo()
{
	applyTextureSnapshot(m_before);
}

void TextureStroke::begin(const TextureAssetRef& texture)
{
	if (m_open)
		return;

	TextureResourceRef resource = texture.resource();
	if (resource.isEmpty())
		return;

	m_texture = texture;
	m_start = takeTextureSnapshot(texture, { 0, 0, resource->getWidth(), resource->getHeight() });
	m_rect = {};
	m_open = true;
}

void TextureStroke::addDab(glm::vec2 centerPixel, float radiusPixels)
{
	if (!m_open)
		return;

	m_rect = unionRect(m_rect, dabRect(centerPixel, radiusPixels, m_texture.resource()));
}

void TextureStroke::end(const std::string& label)
{
	if (!m_open)
		return;

	m_open = false;
	TextureSnapshot start = std::move(m_start);
	m_start = {};

	// A resize erases the old heightmap; the copy fits nothing else
	if (m_texture.isEmpty() || isEmpty(m_rect))
		return;

	TextureSnapshot before = cropSnapshot(start, m_rect);
	TextureSnapshot after = takeTextureSnapshot(m_texture, m_rect);
	CommandStack::Instance().record(std::make_unique<EditTextureCmd>(std::move(before), std::move(after), label));
}
