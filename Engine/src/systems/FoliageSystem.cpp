#include "FoliageSystem.h"

#include "geometry/MeshBuilder.h"
#include "Context.h"
#include "ModelImporter.h"
#include "render/VertexArrayObject.h"
#include "Quad.h"
#include "core/Factory.h"
#include <GL/glew.h>

FoliageSystem::FoliageSystem()
{
	Engine::get()->registerSubSystem<FoliageSystem>(this);
}

bool FoliageSystem::init()
{
	//m_foliageShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");
	m_foliageShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");

	// At the moment i dont have infrastructure to import a mesh and alter its VAO in the same call.
	//auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/grass.obj"); // TODO use single blade model
	auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import("C:/Users/Stav/Downloads/single_grass_blade/scene.gltf");

	//Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_GRASS");
	//Quad::createMesh(meshCollection);
	m_grassBlade = modelInfo.mesh;

	m_grassBlade->getPrimaryMesh()->getVAO()->Bind();

	std::shared_ptr<VertexBufferObject> vbo = std::make_shared<VertexBufferObject>(0, )
	m_grassBlade->getPrimaryMesh()->getVAO()->AttachBuffer()

	

	m_grassBlade->getPrimaryMesh()->getVAO()->Unbind();

	return true;
}

Resource<MeshCollection> FoliageSystem::getGrassBladeMesh()
{
	// TODO change, this is very specific to impl
	return m_grassBlade;
}

void FoliageSystem::setMeshLocations(const std::vector<glm::vec3>& locations)
{
	// TODO change, this is very specific to impl
	//glBindBuffer(GL_ARRAY_BUFFER, m_grassBlade->getPrimaryMesh()->getVAO()->getBufferID());

	//for (int i = 0; i < locations.size(); i++)
	//{
	//	glBufferSubData(GL_ARRAY_BUFFER, i*, sizeof(glm::vec3), locations.data());
	//}

	std::shared_ptr<VertexBufferObject> vbo = std::make_shared<VertexBufferObject>(&(locations[0]), locations.size(), locations.size() * sizeof(glm::vec3)); //when clean will cause issues
	vbo->Bind();

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));
	glVertexAttribDivisor(4, 1);

	m_grassBlade->getPrimaryMesh()->getVAO()->AttachBuffer(vbo, 0); // this will cause issues

	count = locations.size();
}

int FoliageSystem::getCount() const
{
	// TODO change, this is very specific to impl
	return count;
}

Resource<Shader>& FoliageSystem::getFoliageShader()
{
	return m_foliageShader;
}
