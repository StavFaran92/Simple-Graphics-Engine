#include "Terrain.h"
#include "Texture.h"
#include "render/VertexLayout.h"
#include "Assets.h"
#include "MeshBuilder.h"
#include "core/Factory.h"
#include "systems/Logger.h"
#include "CommonTextures.h"
#include "geometry/Grid.h"
#include "Context.h"
#include "Scene.h"
#include "systems/BuiltInMeshes.h"

#include "GL/glew.h"

Entity Terrain::createTerrain(int width, int height, float scale, Resource<Texture> heightMap)
{
	auto terrainEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Terrain");

	auto& terrainComponent = generateTerrain(width, height, scale, heightMap);
	terrainComponent.m_textureCount = 1;

	auto& grassTexture = Texture::importTexture2D(SGE_ROOT_DIR + "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg");
	terrainComponent.setTexture(0, grassTexture);

	terrainEntity.addComponent<Terrain>(terrainComponent);

	return terrainEntity;
}

Terrain Terrain::generateTerrain(int width, int height, float scale, const std::string& heightMapFilepath)
{
	auto heightMap = Texture::importTexture2D(heightMapFilepath);

	return generateTerrain(width, height, scale, heightMap);
}

Terrain Terrain::generateTerrain(int width, int height, float scale, Resource<Texture> heightMap)
{
	auto& meshCollection = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::GRID);//Grid::generateGrid(10, 10, false);

	Terrain terrain;
	terrain.m_heightmap = heightMap;
	terrain.m_scale = scale;
	terrain.m_width = width;
	terrain.m_height = height;
	terrain.m_mesh = meshCollection;

	for (int i = 0; i < MAX_TEXTURE_COUNT; i++)
	{
		TextureBlend blend;
		blend.texture = Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1);
		blend.blend = i * .2f + .2f;
		terrain.m_textureBlends.push_back(blend);
	}

	return terrain; // todo fix
}

Resource<MeshCollection> Terrain::getMesh() const
{
	return m_mesh;
}

float Terrain::getScale() const
{
	return m_scale;
}

Resource<Texture> Terrain::getHeightmap() const
{
	return m_heightmap;
}

int Terrain::getWidth() const
{
	return m_width;
}

int Terrain::getHeight() const
{
	return m_height;
}

void Terrain::setTexture(int index, Resource<Texture> texture)
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

Resource<Texture>& Terrain::getTexture(int index)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture index specified: " + std::to_string(index));
		return Resource<Texture>::empty;
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

float Terrain::getHeightAtPoint(float x, float y) const
{
	
	// offset to match heightmap
	x += m_width / 2;
	y += m_height / 2;

	if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
		return 0.0f;
	}

	// Convert from world space to heightmap space
	float normalizedX = x / m_width * m_heightmap.get()->getWidth();
	float normalizedY = y / m_height * m_heightmap.get()->getHeight();

	// Flip Y axis
	float flippedY = normalizedY;// m_heightmap.get()->getHeight() - 1 - normalizedY;

	// Access heightmap data
	unsigned char* pixels = static_cast<unsigned char*>(m_heightmap.get()->getData().data);
	int stride = m_heightmap.get()->getWidth() * m_heightmap.get()->getData().bpp;

	// Compute floor values
	int floorX = static_cast<int>(floor(normalizedX));
	int floorY = static_cast<int>(floor(flippedY));

	// Offsets within the cell
	float offsetX = normalizedX - floorX;
	float offsetY = flippedY - floorY;

	// Get pixel values
	int indexP0 = floorY * stride + floorX * m_heightmap.get()->getData().bpp;
	int indexP1 = floorY * stride + (floorX + 1) * m_heightmap.get()->getData().bpp;
	int indexP2 = (floorY - 1) * stride + floorX * m_heightmap.get()->getData().bpp;
	int indexP3 = (floorY - 1) * stride + (floorX + 1) * m_heightmap.get()->getData().bpp;

	//     P2  +--------+  P3
	//         |      / |
	//         | T1  /  |
	//         |    /   |
	//         |   /    |
	//         |  /     |
	//         | /   T2 |
	//     P0  |/_______|  P1

	float P0 = pixels[indexP0];
	float P1 = pixels[indexP1];
	float P2 = pixels[indexP2];
	float P3 = pixels[indexP3];

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

	return height;
}