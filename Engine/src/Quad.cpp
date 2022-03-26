#include "Quad.h"

#include "Mesh.h"

static const float vertices[] = {
	// positions          // colors           // texture coords
 1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   // top right
 1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   // bottom right
-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
-1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f    // top left 
};

static const unsigned int indices[] = {
	3, 2, 1,   // first triangle
	1, 0, 3    // second triangle
};

Quad* Quad::generateQuad()
{
	auto quad = new Quad();

	auto mesh = generateMesh();

	auto texturediff = Texture::LoadTextureFromFile("Resources\\Textures\\template.png");
	texturediff->SetType(Constants::g_textureDiffuse);

	auto textureSpec = Texture::LoadTextureFromFile("Resources\\Textures\\template.png");
	textureSpec->SetType(Constants::g_textureSpecular);

	mesh->addTexture(texturediff);
	mesh->addTexture(textureSpec);

	std::shared_ptr<Material> material = std::make_shared<Material>(32.0f);
	quad->UseMaterial(material);

	quad->m_meshes.push_back(mesh);

	return quad;
}

std::shared_ptr<Mesh> Quad::generateMesh()
{
	auto mesh = std::make_shared<Mesh>();
	Mesh::VerticesLayout layout;
	layout.numOfVertices = 4;
	layout.entries.emplace_back(Mesh::LayoutAttributes::Positions, 3);
	layout.entries.emplace_back(Mesh::LayoutAttributes::Normals, 3);
	layout.entries.emplace_back(Mesh::LayoutAttributes::Texcoords, 2);
	mesh->setRawVertices((float*)vertices, layout);
	mesh->setRawIndices((unsigned int*)indices, sizeof(indices) / sizeof(unsigned int));
	mesh->build();

	return mesh;
}