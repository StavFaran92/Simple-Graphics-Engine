#pragma once

#include <memory>
#include <vector>

#include "memory/Resource.h"
#include "render/Shader.h"
#include "texture/Texture.h"

class TerrainMaterial
{
public:
	TerrainMaterial() = default;
	~TerrainMaterial() = default;

	const std::vector<Resource<Texture>>& getAllTextures() const;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_textures);
	}

protected:
	std::vector<Resource<Texture>> m_textures;
};
