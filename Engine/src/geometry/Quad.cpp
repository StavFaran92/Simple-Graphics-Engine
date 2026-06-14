#include "geometry/Quad.h"

#include "geometry/Mesh.h"
#include "geometry/MeshBuilder.h"
#include "render/VertexLayout.h"
#include "geometry/ModelImporter.h"
#include "runtime/Context.h"

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

std::shared_ptr<Mesh> Quad::createMesh()
{
	//ModelImporter::ModelImportSettings settings;
	//settings.name = "SGE_QUAD_MESH";
	//settings.isTransient = true;
	//return Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR "Resources/Engine/Meshes/plane.gltf", settings).mesh;

	VertexLayout layout;
	layout.numOfVertices = 4;
	layout.attribs.emplace_back(LayoutAttribute::Positions);
	layout.attribs.emplace_back(LayoutAttribute::Normals);
	layout.attribs.emplace_back(LayoutAttribute::Texcoords);

	auto mesh = MeshBuilder(MeshType::StaticMesh)
		.addRawVertices((float*)vertices, layout)
		.addIndices((unsigned int*)indices, sizeof(indices) / sizeof(unsigned int))
		.build();

	return mesh;
}

