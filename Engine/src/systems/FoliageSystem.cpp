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
	ModelImporter::ModelImportSettings settings;
	settings.isTransient = true;
	auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/scene.gltf", settings);

	//Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_GRASS");
	//Quad::createMesh(meshCollection);
	m_grassBlade = modelInfo.mesh;

	glGenBuffers(1, &ssbo);

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

	//VertexLayout layout;
	//layout.attribs.push_back(LayoutAttribute::InstancePos);

	//auto vbo = VertexBufferObject::createRaw(&(locations[0]), locations.size(), locations.size() * sizeof(glm::vec3), layout);
	//m_grassBlade->getPrimaryMesh()->getVAO()->attachBuffer(vbo, 0);
	//m_grassBlade->getPrimaryMesh()->getVAO()->build(); 
	//glVertexAttribDivisor(4, 1);

	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec3), locations.data(), GL_DYNAMIC_DRAW);

	//std::shared_ptr<VertexBufferObject> vbo = std::make_shared<VertexBufferObject>(&(locations[0]), locations.size(), locations.size() * sizeof(glm::vec3)); //when clean will cause issues
	//vbo->Bind();

	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));

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

unsigned int FoliageSystem::getSSBO() const
{
	return ssbo;
}
