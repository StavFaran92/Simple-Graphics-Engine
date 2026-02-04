#include "geometry/Grid.h"

#include "core/Factory.h"
#include "geometry/MeshBuilder.h"
#include "geometry/MeshExporter.h"
#include "memory/Assets.h"

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

void Grid::generateGrid(ResourceWrapper<Model>& meshCollection, int x, int y)
{
	int xRez = x;
	int yRez = y;

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
}
