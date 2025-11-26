#pragma once

#include <string>
#include <vector>

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"

class Entity;

static const int MAX_TEXTURE_COUNT = 4;

struct TextureBlend
{
	AssetWrapper<Texture> texture;
	float blend = 0;
	float scaleX = 1;
	float scaleY = 1;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(texture, blend, scaleX, scaleY);
	}
};

class EngineAPI Terrain : public Component
{
public:
	static Terrain generateTerrain(int width, int height, float scale, const std::string& heightMapFilepath);
	static Terrain generateTerrain(int width, int height, float scale, AssetWrapper<Texture> heightMap);

	static Entity createTerrain(int width, int height, float scale, AssetWrapper<Texture> heightMap);


	Terrain() = default;

	ResourceWrapper<MeshCollection> getMesh() const;
	ResourceWrapper<Texture> getHeightmap() const;
	float getHeightAtPoint(float x, float y) const;

	float getScale() const;
	int getWidth() const;
	int getHeight() const;

	void setTexture(int index, AssetWrapper<Texture> texture);
	void setTextureScaleX(int index, float scaleX);
	void setTextureScaleY(int index, float scaleY);
	void setTextureBlend(int index, float val);

	AssetWrapper<Texture>& getTexture(int index);
	float getTextureBlend(int index) const;
	glm::vec2 getTextureScale(int index) const;

	int getTextureCount() const;

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_mesh);
		SERIALIZED_MEMBER(m_heightmap);
		SERIALIZED_MEMBER(m_width);
		SERIALIZED_MEMBER(m_height);
		SERIALIZED_MEMBER(m_scale);
		SERIALIZED_MEMBER(m_material);
	}


	
	AssetWrapper<Texture> m_heightmap;
	

	int m_width = 100;
	int m_height = 100;
	int m_scale = 1;
	int m_textureCount = 0;

	std::vector<TextureBlend> m_textureBlends {  };
	//std::vector<Resource<Texture>> m_textures{  };
	//std::vector<float> m_blends{ };
	AssetWrapper<Material> m_material;

private:
	AssetWrapper<MeshCollection> m_mesh;
	//std::shared_ptr<TextureArray> m_textures;

	
};

REGISTER_COMPONENT(Terrain)
