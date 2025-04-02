#include "Quad.h"

#include "Mesh.h"
#include "MeshBuilder.h"
#include "VertexLayout.h"
#include "ModelImporter.h"
#include "Context.h"

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

Resource<MeshCollection> Quad::createMesh()
{
	ModelImporter::ModelImportSettings settings;
	settings.name = "SGE_QUAD_MESH";
	return Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/plane.gltf", settings).mesh;
	//VertexLayout layout;
	//layout.numOfVertices = 4;
	//layout.attribs.emplace_back(LayoutAttribute::Positions);
	//layout.attribs.emplace_back(LayoutAttribute::Normals);
	//layout.attribs.emplace_back(LayoutAttribute::Texcoords);

	//return MeshBuilder::builder()
	//	.addRawVertices((float*)vertices, layout)
	//	.addIndices((unsigned int*)indices, sizeof(indices) / sizeof(unsigned int))
	//	.build();
}

