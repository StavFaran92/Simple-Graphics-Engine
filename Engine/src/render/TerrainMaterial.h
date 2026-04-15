#pragma once

#include <memory>
#include <vector>

#include "memory/ResourceRef.h"
#include "render/Shader.h"
#include "texture/Texture.h"

class TerrainMaterial
{
public:
	TerrainMaterial() = default;
	~TerrainMaterial() = default;

	const std::vector<TextureResourceRef>& getAllTextures() const;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_textures);
	}

protected:
	std::vector<TextureResourceRef> m_textures;
};
