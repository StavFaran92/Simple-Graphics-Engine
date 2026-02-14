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

Entity Terrain::createTerrain(int width, int height)
{
	auto terrainEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Terrain");
	terrainEntity.addComponent<Terrain>(Terrain::createTerrainComponent(width, height));
	return terrainEntity;
}

void Terrain::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	if (auto tc = std::dynamic_pointer_cast<Terrain>(c))
	{
		auto& terrain = entityHandler.addComponent<Terrain>(*tc);
		terrain.build();
	}
}

AssetHandle<TextureAsset> Terrain::generateHeightmap(int width, int height)
{
	m_heightDataCPU = std::vector<float>(width * height, 0.0f);

	TextureData tData;
	tData.target = TextureTarget::TEXTURE_2D;
	tData.width = width;
	tData.height = height;
	tData.channels = 1;
	tData.data = m_heightDataCPU.data();
	tData.internalFormat = TextureInternalFormat::R32F;
	tData.format = TextureFormat::RED;
	tData.type = TextureType::FLOAT;
	tData.textureName = "SGE_TERRAIN_HEIGHTMAP";
	tData.filter = TextureFilter::Linear;
	tData.wrap = TextureWrap::Clamp;

	auto texture = Texture::createTexture(tData);

	AssetCreateDescriptor desc;
	desc.aType = AssetType::TEXTURE;
	desc.name = "SGE_TERRAIN_HEIGHTMAP";
	desc.isEngineOwned = true;
	//desc.attributes = texture->getTextureAssetAttributes().toMap();
	TextureLoadDescriptor* resourceDesc = new TextureLoadDescriptor();
	resourceDesc->usage = TextureSemantic::Heightmap;

	desc.resourceLoadDescriptor = resourceDesc;
	auto heightmap = TextureAsset::create(texture, desc);
	//auto heightmap = Engine::get()->getSubSystem<Assets>()->createAsset(texture, desc).as<TextureAsset>();

	return heightmap;
}

Terrain Terrain::createTerrainComponent(int width, int height)
{
	auto& meshCollection = BuiltInAssets::getByName<MeshGroupAsset>(SGE_MESH_GRID);//Grid::generateGrid(10, 10, false);

	Terrain terrain;
	terrain.m_heightmap = terrain.generateHeightmap(width, height);
	terrain.m_scale = 1;
	terrain.m_width = width;
	terrain.m_height = height;
	terrain.m_mesh = meshCollection;

	terrain.m_textureCount = 1;

	for (int i = 0; i < MAX_TEXTURE_COUNT; i++)
	{
		TextureBlend blend;
		blend.texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE);
		blend.blend = i * .2f + .2f;
		terrain.m_textureBlends.push_back(blend);
	}


	terrain.m_material = BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_TERRAIN_DEFAULT);
	return terrain; // todo fix
}

ResourceWrapper<MeshGroup> Terrain::getMesh() const
{
	return m_mesh.resource();
}

float Terrain::getScale() const
{
	return m_scale;
}

void Terrain::setHeightmap(AssetHandle<TextureAsset> heightmap)
{
	m_heightmap = heightmap;

	build();
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

void Terrain::setTexture(int index, AssetHandle<TextureAsset> texture)
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

AssetHandle<TextureAsset>& Terrain::getTexture(int index)
{
	if (index > m_textureBlends.size() - 1)
	{
		logWarning("Invalid texture index specified: " + std::to_string(index));
		return AssetHandle<TextureAsset>::empty;
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
	const std::vector<float>& pixels,
	int floorX, int floorY,
	int stride, int width, int height
) {
	auto getIndex = [&](int x, int y) -> int {
		return (y * stride + x);
		};

	int indexP0 = getIndex(floorX, floorY);
	int indexP1 = getIndex(floorX + 1, floorY);
	int indexP2 = getIndex(floorX, floorY + 1);
	int indexP3 = getIndex(floorX + 1, floorY + 1);

	int totalBytes = width * height;

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
	//unsigned char* pixels = static_cast<unsigned char*>(m_heightmap.resource().get()->getData().data);
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
		m_heightDataCPU,
		floorX, floorY,
		stride,
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
	float height = (P0 + lerpX + lerpY) * m_scale;

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

	m_heightmap.erase();

	m_heightmap = generateHeightmap(newW, newH);

	if(m_foliageField.isActive)
		m_foliageField.resize(newW, newH);
}

void Terrain::setPixel(int x, int y, unsigned char value)
{

}

void Terrain::build()
{
	syncHeightmap();
	buildFoliage();
}

void Terrain::syncHeightmap()
{
	if (m_heightmap.isEmpty())
		return;

	auto res = m_heightmap.resource();

	// If CPU buffer size is not the same as GPU buffer size reallocate
	if (m_heightDataCPU.size() != res->getWidth() * res->getHeight())
	{
		m_heightDataCPU = std::vector<float>(res->getWidth() * res->getHeight(), 0.0f);
	}

	res->bind();

	glGetTexImage(
		GL_TEXTURE_2D,
		0,
		toGL(res->getData().format),
		toGL(res->getData().type),
		m_heightDataCPU.data()
	);

	m_heightmap.resource()->getData().data = m_heightDataCPU.data();
	m_heightmap.makeDirty();
}

RayHit Terrain::raycast(const Ray& ray, float maxDistance)
{
	RayHit result;

	if (ray.direction.y >= 0.0f)
		return result;

	float dirXZ = glm::length(glm::vec2(ray.direction.x, ray.direction.z));
	if (dirXZ < 1e-5f)
		return result;

	AABB bounds = getAABB();

	float tStart, tEnd;
	if (!Math3D::RayIntersectXZBounds(ray, bounds, tStart, tEnd))
		return result;

	tStart = std::max(tStart, 0.0f);
	tEnd = std::min(tEnd, maxDistance);

	constexpr float STEP_XZ = 0.5f;
	float stepT = STEP_XZ / dirXZ;

	float t = tStart;
	float prevDiff = 0.0f;
	bool first = true;

	while (t <= tEnd) {
		glm::vec3 p = ray.origin + ray.direction * t;

		float h;
		if (!getHeightAtPoint(p.x, p.z, h)) {
			t += stepT;
			first = true;
			continue;
		}

		float diff = p.y - h;

		if (!first && prevDiff > 0.0f && diff <= 0.0f) {
			// refine hit
			float t0 = t - stepT;
			float t1 = t;

			for (int i = 0; i < 5; ++i) {
				float tm = 0.5f * (t0 + t1);
				glm::vec3 pm = ray.origin + ray.direction * tm;

				float hm;
				getHeightAtPoint(pm.x, pm.z, hm);

				if (pm.y > hm)
					t0 = tm;
				else
					t1 = tm;
			}

			float tHit = 0.5f * (t0 + t1);

			result.hit = true;
			result.t = tHit;
			result.position = ray.origin + ray.direction * tHit;

			// normal
			const float eps = 0.1f;
			float hL, hR, hD, hU;
			getHeightAtPoint(result.position.x - eps, result.position.z, hL);
			getHeightAtPoint(result.position.x + eps, result.position.z, hR);
			getHeightAtPoint(result.position.x, result.position.z - eps, hD);
			getHeightAtPoint(result.position.x, result.position.z + eps, hU);

			result.normal = glm::normalize(glm::vec3(
				hL - hR,
				2.0f * eps,
				hD - hU
			));

			return result;
		}

		prevDiff = diff;
		first = false;
		t += stepT;
	}

	return result;
}
