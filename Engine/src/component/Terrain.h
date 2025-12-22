#pragma once

#include <string>
#include <vector>

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"

#include "component/FoliageField.h"

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
	bool getHeightAtPoint(float x, float y, float& outHeight) const;

	float getScale() const;
	int getWidth() const;
	int getHeight() const;

	void setPixel(int x, int y, unsigned char value);

	void setTexture(int index, AssetWrapper<Texture> texture);
	void setTextureScaleX(int index, float scaleX);
	void setTextureScaleY(int index, float scaleY);
	void setTextureBlend(int index, float val);

	AssetWrapper<Texture>& getTexture(int index);
	float getTextureBlend(int index) const;
	glm::vec2 getTextureScale(int index) const;

	AABB getAABB() const;

	int getTextureCount() const;

	void buildFoliage();

	void resize(int newW, int newH);

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_mesh);
		SERIALIZED_MEMBER(m_heightmap);
		SERIALIZED_MEMBER(m_width);
		SERIALIZED_MEMBER(m_height);
		SERIALIZED_MEMBER(m_scale);
		SERIALIZED_MEMBER(m_material);
		SERIALIZED_MEMBER(m_foliageField);
	}


	
	AssetWrapper<Texture> m_heightmap;
	


	int m_scale = 1;
	int m_textureCount = 0;

	std::vector<TextureBlend> m_textureBlends {  };

	AssetWrapper<Material> m_material;

	FoliageField m_foliageField;

private:
	int m_width = 100;
	int m_height = 100;

	AssetWrapper<MeshCollection> m_mesh;
	//std::shared_ptr<TextureArray> m_textures;

	
};

REGISTER_COMPONENT(Terrain)
