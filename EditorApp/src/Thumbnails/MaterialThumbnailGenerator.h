#pragma once

#include "ThumbnailGenerator.h"
#include "PreviewRenderer.h"

// Renders the material on the built-in sphere.
// Only PBR render modes (Opaque, Transparent) are rendered; the rest fall back to the type icon.
class MaterialThumbnailGenerator : public IThumbnailGenerator
{
public:
	bool generate(const AssetRecord& aInfo, const TextureResourceRef& target) override;

private:
	PreviewRenderer m_renderer;
};
