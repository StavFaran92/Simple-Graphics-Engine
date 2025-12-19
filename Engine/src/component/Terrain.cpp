#include "component/Terrain.h"
#include "texture/Texture.h"
#include "render/VertexLayout.h"
#include "memory/Assets.h"
#include "geometry/MeshBuilder.h"
#include "core/Factory.h"
#include "core/Logger.h"

#include "geometry/Grid.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"

#include "memory/BuiltInAssets.h"

#include "GL/glew.h"

Entity Terrain::createTerrain(int width, int height, float scale, AssetWrapper<Texture> heightMap)
{
	auto terrainEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Terrain");

	auto& terrainComponent = generateTerrain(width, height, scale, heightMap);
	terrainComponent.m_textureCount = 1;

	auto& grassTexture = BuiltInAssets::getByName<Texture>("SGE_TEXTURE_GRASS");
	terrainComponent.setTexture(0, grassTexture);

	terrainEntity.addComponent<Terrain>(terrainComponent);

	return terrainEntity;
}

void Terrain::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	if (auto tc = std::dynamic_pointer_cast<Terrain>(c))
	{
		entityHandler.addComponent<Terrain>(*tc);
		tc->buildFoliage();
	}
}

Terrain Terrain::generateTerrain(int width, int height, float scale, const std::string& heightMapFilepath)
{
	// Ensure height sampling does not wrap at borders so terrain edges use edge heights
	Texture::TextureAssetDescriptor settings;
	settings.params[GL_TEXTURE_WRAP_S] = GL_CLAMP_TO_EDGE;
	settings.params[GL_TEXTURE_WRAP_T] = GL_CLAMP_TO_EDGE;
	settings.params[GL_TEXTURE_MIN_FILTER] = GL_LINEAR;
	settings.params[GL_TEXTURE_MAG_FILTER] = GL_LINEAR;
	auto heightMap = Texture::import(heightMapFilepath, settings);

	return generateTerrain(width, height, scale, heightMap);
}

Terrain Terrain::generateTerrain(int width, int height, float scale, AssetWrapper<Texture> heightMap)
{
	auto& meshCollection = BuiltInAssets::getByName<MeshCollection>(SGE_MESH_GRID);//Grid::generateGrid(10, 10, false);

	Terrain terrain;
	terrain.m_heightmap = heightMap;
	terrain.m_scale = scale;
	terrain.m_width = width;
	terrain.m_height = height;
	terrain.m_mesh = meshCollection;

	for (int i = 0; i < MAX_TEXTURE_COUNT; i++)
	{
		TextureBlend blend;
		blend.texture = BuiltInAssets::getByName<Texture>(SGE_TEXTURE_WHITE);
		blend.blend = i * .2f + .2f;
		terrain.m_textureBlends.push_back(blend);
	}


	terrain.m_material = BuiltInAssets::getByName<Material>(SGE_MATERIAL_TERRAIN_DEFAULT);
	return terrain; // todo fix
}

ResourceWrapper<MeshCollection> Terrain::getMesh() const
{
	return m_mesh.resource();
}

float Terrain::getScale() const
{
	return m_scale;
}

ResourceWrapper<Texture> Terrain::getHeightmap() const
{
	return m_heightmap.resource();
}

int Terrain::getWidth() const
{
	return m_width;
}

int Terrain::getHeight() const
{
	return m_height;
}

void Terrain::setTexture(int index, AssetWrapper<Texture> texture)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture specified: " + std::to_string(index));
		return;
	}

	m_textureBlends[index].texture = texture;
}

void Terrain::setTextureScaleX(int index, float scaleX)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture index specified: " + std::to_string(index));
		return;
	}

	m_textureBlends.at(index).scaleX = scaleX;
}

void Terrain::setTextureScaleY(int index, float scaleY)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture index specified: " + std::to_string(index));
		return;
	}

	m_textureBlends.at(index).scaleY = scaleY;
}

void Terrain::setTextureBlend(int index, float val)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture blend specified: " + std::to_string(index));
		return;
	}

	m_textureBlends[index].blend = val;
}

AssetWrapper<Texture>& Terrain::getTexture(int index)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture index specified: " + std::to_string(index));
		return AssetWrapper<Texture>::empty;
	}

	return m_textureBlends.at(index).texture;
}

float Terrain::getTextureBlend(int index) const
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture blend specified: " + std::to_string(index));
		return 0;
	}

	return m_textureBlends.at(index).blend;
}

glm::vec2 Terrain::getTextureScale(int index) const
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture blend specified: " + std::to_string(index));
		return {};
	}

	auto& textureBlend = m_textureBlends.at(index);
	return { textureBlend.scaleX, textureBlend.scaleY };
}

int Terrain::getTextureCount() const
{
	return m_textureCount;
}

std::array<float, 4> getCornersSafe(
	const unsigned char* pixels,
	int floorX, int floorY,
	int stride, int bpp,
	int width, int height
) {
	auto getIndex = [&](int x, int y) -> int {
		return (y * stride + x) * bpp;
		};

	int indexP0 = getIndex(floorX, floorY);
	int indexP1 = getIndex(floorX + 1, floorY);
	int indexP2 = getIndex(floorX, floorY + 1);
	int indexP3 = getIndex(floorX + 1, floorY + 1);

	int totalBytes = width * height * bpp;

	auto safe = [&](int idx) -> float {
		if (idx >= 0 && idx < totalBytes) {
			return pixels[idx];
		}
		else {
			if (indexP0 >= 0)
			{
				return pixels[indexP0];
			}
			else
			{
				return 0.f;
			}
		}
		};

	return {
		safe(indexP0),
		safe(indexP1),
		safe(indexP2),
		safe(indexP3)
	};
}

bool Terrain::getHeightAtPoint(float x, float y, float& outHeight) const
{
	
	// offset to match heightmap
	x += m_width / 2.f;
	y += m_height / 2.f;

	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return false;
	}

	// Convert from world space to heightmap space
	float normalizedX = x / m_width * m_heightmap.resource().get()->getWidth();
	float normalizedY = y / m_height * m_heightmap.resource().get()->getHeight();

	normalizedX -= .5;
	normalizedY -= .5;

	// Flip Y axis
	float flippedY = normalizedY;// m_heightmap.get()->getHeight() - 1 - normalizedY;

	// Access heightmap data
	unsigned char* pixels = static_cast<unsigned char*>(m_heightmap.resource().get()->getData().data);
	int stride = m_heightmap.resource().get()->getWidth();

	// Compute floor values
	int floorX = static_cast<int>(floor(normalizedX));
	int floorY = static_cast<int>(floor(flippedY));

	// Offsets within the cell
	float offsetX = normalizedX - floorX;
	float offsetY = flippedY - floorY;

	// Get pixel values

	//     P0  +--------+  P1
	//         |\       |
	//         | \   T2 |
	//         |  \     |
	//         |   \    |
	//         |    \   |
	//         |  T1 \  |
	//         |      \ |
	//     P2  |_______\|  P3

	auto [P0, P1, P2, P3] = getCornersSafe(
		pixels,
		floorX, floorY,
		stride,
		m_heightmap.resource().get()->getBitDepth(),
		m_heightmap.resource().get()->getWidth(),
		m_heightmap.resource().get()->getHeight()
	);

	float lerpX = 0.0f;
	float lerpY = 0.0f;

	// lerp results using neighbor pixels
	if (offsetY > offsetX) // T1
	{
		lerpX = (P3 - P2) * offsetX;
		lerpY = (P2 - P0) * offsetY;
	}
	else // T2
	{
		lerpX = (P1 - P0) * offsetX;
		lerpY = (P3 - P1) * offsetY;
	}

	// sum results
	float height = (P0 + lerpX + lerpY) / 255.f * m_scale;

	outHeight = height;

	return true;
}

void Terrain::buildFoliage()
{
	m_foliageField.build(m_width, m_height);
}

AABB Terrain::getAABB() const
{
	return AABB::createFromCenterExtents(glm::vec3(0.f), glm::vec3(m_width, m_scale, m_height)); // todo return member
}

void Terrain::resize(int newW, int newH)
{
	m_width = newW;
	m_height = newH;

	m_foliageField.resize(newW, newH);
}