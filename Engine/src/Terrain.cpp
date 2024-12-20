#include "Terrain.h"
#include "Texture.h"
#include "VertexLayout.h"
#include "Assets.h"
#include "MeshBuilder.h"
#include "Factory.h"
#include "MeshExporter.h"
#include "Logger.h"
#include "CommonTextures.h"

#include "GL/glew.h"

aiScene* generateScene(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
{
	// Create a new mesh
	aiMesh* mesh = new aiMesh();
	mesh->mNumVertices = vertices.size();
	mesh->mVertices = new aiVector3D[mesh->mNumVertices];
	mesh->mNormals = new aiVector3D[mesh->mNumVertices];
	mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
	mesh->mNumUVComponents[0] = 2;

	// Set vertices
	for (unsigned int i = 0; i < mesh->mNumVertices / 5; ++i) 
	{
		mesh->mVertices[i] = aiVector3D(vertices[i * 5 + 0], vertices[i * 5 + 1], vertices[i * 5 + 2]);
		mesh->mTextureCoords[0][i] = aiVector3D(vertices[i * 5 + 3], vertices[i * 5 + 4], 0.0f);
	}

	mesh->mNumFaces = indices.size() / 4;
	mesh->mFaces = new aiFace[mesh->mNumFaces];

	// Set faces
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		mesh->mFaces[i].mNumIndices = 4;
		mesh->mFaces[i].mIndices = new unsigned int[4] { indices[i * 4 + 0], indices[i * 4 + 1], indices[i * 4 + 2], indices[i * 4 + 3] };
	}

	mesh->mPrimitiveTypes = aiPrimitiveType_POLYGON;

	// Create a new scene
	aiScene* scene = new aiScene();
	scene->mNumMeshes = 1;
	scene->mMeshes = new aiMesh * [1] { mesh };
	scene->mNumMaterials = 1;
	scene->mMaterials = new aiMaterial * [1] { new aiMaterial() };
	scene->mRootNode = new aiNode();
	scene->mRootNode->mNumMeshes = 1;
	scene->mRootNode->mMeshes = new unsigned int[1] { 0 };

	return scene;
}

Terrain Terrain::generateTerrain(int width, int height, float scale, const std::string& heightMapFilepath)
{
	auto heightMap = Engine::get()->getSubSystem<Assets>()->importTexture2D(heightMapFilepath);

	return generateTerrain(width, height, scale, heightMap);
}

Terrain Terrain::generateTerrain(int width, int height, float scale, Resource<Texture> heightMap)
{
	int xRez = 10;
	int yRez = 10;

	// vertex generation
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	int vertexCount = (xRez + 1) * (yRez + 1);

	vertices.reserve(vertexCount * 5);
	indices.reserve(xRez * yRez * 4);

	// Vertex generation
	for (int i = 0; i <= yRez; i++) 
	{ 
		for (int j = 0; j <= xRez; j++) 
		{ 
			// Vertex positions
			float xPos = (j / (float)xRez) - 0.5f;  // Normalize to [-0.5, 0.5]
			float zPos = (i / (float)yRez) - 0.5f;  // Normalize to [-0.5, 0.5]
			float yPos = 0.0f;  // Flat terrain

			// Texture coordinates
			float u = j / (float)xRez;
			float v = i / (float)yRez;

			// Add position and texture coordinate to vertex list
			vertices.push_back(xPos);  // x
			vertices.push_back(yPos);  // y
			vertices.push_back(zPos);  // z
			vertices.push_back(u);     // u
			vertices.push_back(v);     // v
		}
	}

	// Index generation for the grid
	for (int i = 0; i < yRez; i++) 
	{
		for (int j = 0; j < xRez; j++) 
		{
			int topLeft = i * (xRez + 1) + j;
			int topRight = topLeft + 1;
			int bottomLeft = (i + 1) * (xRez + 1) + j;
			int bottomRight = bottomLeft + 1;

			// Quad indices
			indices.push_back(topLeft);
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}


	Resource<MeshCollection> meshCollection = Factory<MeshCollection>::create();

	auto mesh = std::make_shared<Mesh>();

	VertexLayout layout;
	layout.attribs.push_back(LayoutAttribute::Positions);
	layout.attribs.push_back(LayoutAttribute::Texcoords);
	layout.numOfVertices = vertexCount;
	layout.build();

	MeshBuilder::builder()
		.addRawVertices(vertices.data(), layout)
		.addIndices(indices)
		.build(*mesh.get());

	meshCollection.get()->addMesh(mesh);

	aiScene* scene = generateScene(vertices, indices);

	MeshExporter::exportMesh(meshCollection, scene);

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
	if (x < 0 || x >= m_width - 1 || y < 0 || y >= m_height - 1)
	{
		return 0.0f;
	}

	// convert from world space to heightmap space
	float normalizedX = x / m_width * m_heightmap.get()->getWidth();
	float normalizedY = y / m_height * m_heightmap.get()->getHeight();

	// X and Y are bottom->up, while texture data is up->bottom
	float flippedY = m_heightmap.get()->getHeight() - normalizedY;

	//     P1  +--------+  P2
	//         |      / |
	//         | T1  /  |
	//         |    /   |
	//         |   /    |
	//         |  /     |
	//         | /   T2 |
	//     P0  |/_______|  P3

	// TODO optimize
	m_heightmap.get()->bind();
	void* pixels = malloc(m_heightmap.get()->getWidth() * m_heightmap.get()->getHeight() * m_heightmap.get()->getData().bpp);
	glGetTexImage(GL_TEXTURE_2D, 0, m_heightmap.get()->getData().format, m_heightmap.get()->getData().type, pixels);

	if (!pixels)
	{
		logError("Fetched Pixel is NULL");
		return 0.0f;
	}

	int stride = m_heightmap.get()->getWidth() * m_heightmap.get()->getData().bpp;

	float floorX = floor(normalizedX);
	float floorY = floor(flippedY);

	float offsetX = normalizedX - floor(normalizedX);
	float offsetY = flippedY - floor(flippedY);

	float P0 = static_cast<unsigned char*>(pixels)[static_cast<int>(floorY * stride + floorX)];
	float P2 = static_cast<unsigned char*>(pixels)[static_cast<int>((floorY - 1) * stride + floorX + m_heightmap.get()->getData().bpp)];

	float Pn = 0.0f;
	if(normalizedY > normalizedX)
		Pn = static_cast<unsigned char*>(pixels)[static_cast<int>((floorY - 1) * stride + floorX)]; // T1
	else
		Pn = static_cast<unsigned char*>(pixels)[static_cast<int>(floorY * stride + floorX + m_heightmap.get()->getData().bpp)]; // T2

	// calculate offset inside pixel
	float t2hX = m_width / m_heightmap.get()->getWidth();
	float t2hY = m_height / m_heightmap.get()->getHeight();
	

	// lerp results using neighbor pixels
	float lerpX = P0 * (1.0f - offsetX) + offsetX * Pn - P0;
	float lerpY = Pn * (1.0f - offsetY) + offsetY * P2 - Pn;

	// sum results
	float output = (P0 + lerpX + lerpY) / 255.f * m_scale;

	return output;
}