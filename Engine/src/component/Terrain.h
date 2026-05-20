#pragma once

#include <vector>

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"

#include "component/FoliageField.h"

#include "utils/Math3D.h"
#include "geometry/Model.h"
#include "render/TerrainLayer.h"

class Entity;

static const int MAX_TEXTURE_COUNT = 4;

struct TextureBlend
{
	TextureAssetRef texture;
	float blend = 0;
	float scaleX = 1;
	float scaleY = 1;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(texture, blend, scaleX, scaleY);
	}
};

struct LayerMask
{
	std::string name;
	TextureAssetRef mask;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(mask);
	}
};

class EngineAPI Terrain : public Component
{
public:
	static inline const int MAX_LAYER_COUNT = 3;

	Terrain() = default;

	std::string getName() override { return "Terrain"; }

	void postLoad(SceneResourceRef& scene ) override;

	static Entity createTerrain(int width, int height);

	ModelResourceRef getMesh() const;

	void setHeightmap(TextureAssetRef heightmap);
	TextureResourceRef getHeightmap() const;

	bool getHeightAtPoint(float x, float y, float& outHeight) const;

	float getScale() const;
	int getWidth() const;
	int getHeight() const;

	void setPixel(int x, int y, unsigned char value);

	void setTexture(int index, TextureAssetRef texture);
	void setTextureScaleX(int index, float scaleX);
	void setTextureScaleY(int index, float scaleY);
	void setTextureBlend(int index, float val);

	TextureAssetRef& getTexture(int index);
	float getTextureBlend(int index) const;
	glm::vec2 getTextureScale(int index) const;

	AABB getAABB() const;

	int getTextureCount() const;

	void buildFoliage();

	void resize(int newW, int newH);
	
	void build();

	// Sync texture GPU data to the CPU data
	void syncHeightmap();

	RayHit raycast(const Ray& ray, float maxDistance = 10000.0f);

	void addLayer();
	void removeLayer(int index);
	LayerMask getLayer(int index);
	const int getLayerCount() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(m_mesh);
		SERIALIZED_MEMBER(m_heightmap);
		SERIALIZED_MEMBER(m_width);
		SERIALIZED_MEMBER(m_height);
		SERIALIZED_MEMBER(m_scale);
		SERIALIZED_MEMBER(m_material);
		SERIALIZED_MEMBER(m_foliageField);
		SERIALIZED_MEMBER(m_layerMasks);
	}

	float m_scale = 1.0f;
	int m_textureCount = 0;

	std::vector<TextureBlend> m_textureBlends {  };

	MaterialAssetRef m_material;

	FoliageField m_foliageField;

protected:
	std::vector<AssetRef<Asset>> gatherDependenciesInternal() const override;

private:
	static Terrain createTerrainComponent(int width, int height);
	TextureAssetRef generateHeightmap(int width, int height);
	std::array<float, 4> getCornersSafe(
		int floorX, int floorY,
		int stride, int width, int height
	) const;
private:
	int m_width = 1000;
	int m_height = 1000;

	TextureAssetRef m_heightmap;
	ModelAssetRef m_mesh;

	std::vector<LayerMask> m_layerMasks;

};

REGISTER_COMPONENT(Terrain)
