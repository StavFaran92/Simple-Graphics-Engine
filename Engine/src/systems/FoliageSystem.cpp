#include "FoliageSystem.h"

#include "geometry/MeshBuilder.h"
#include "Context.h"
#include "ModelImporter.h"
#include "render/VertexArrayObject.h"
#include <GL/glew.h>

FoliageSystem::FoliageSystem()
{
	Engine::get()->registerSubSystem<FoliageSystem>(this);
}

bool FoliageSystem::init()
{
	m_foliageShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");

	// At the moment i dont have infrastructure to import a mesh and alter its VAO in the same call.
	//auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/grass.obj"); // TODO use single blade model
	auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import("C:/Users/Stav/Downloads/grass/source/GrassPatch.fbx");
	m_grassBlade = modelInfo.mesh;

	m_grassBlade->getPrimaryMesh()->getVAO()->Bind();

	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);

	m_grassBlade->getPrimaryMesh()->getVAO()->Unbind();

	return true;
}

Resource<MeshCollection> FoliageSystem::getGrassBladeMesh()
{
	// TODO change, this is very specific to impl
	return m_grassBlade;
}

void FoliageSystem::setMeshLocations(const std::vector<glm::mat4>& matrices)
{
	// TODO change, this is very specific to impl
	glBindBuffer(GL_ARRAY_BUFFER, m_grassBlade->getPrimaryMesh()->getVAO()->getBufferID());
	glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), matrices.data(), GL_STATIC_DRAW);

	count = matrices.size();
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
