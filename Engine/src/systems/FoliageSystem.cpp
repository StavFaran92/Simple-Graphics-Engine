#include "systems/FoliageSystem.h"

#include "geometry/MeshBuilder.h"
#include "runtime/Context.h"
#include "geometry/ModelImporter.h"
#include "render/VertexArrayObject.h"
#include "geometry/Quad.h"
#include "core/Factory.h"
#include "render/Graphics.h"
#include "render/RenderCommand.h"
#include <GL/glew.h>
#include "core/Random.h"

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

	glGenBuffers(1, &m_foliageChunksSSBO);
	glGenBuffers(1, &m_visibleFoliageChunksSSBO);
	glGenBuffers(1, &m_finalFoliageLocationsSSBO);

	m_frustumCullComputeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FrustumCullComputeShader.glsl");
	m_populateGrassComputeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PopulateFoliageComputeShader.glsl");

	glGenBuffers(1, &m_atomicCounterBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterBuffer);

	glGenBuffers(1, &m_frustumUBO);

	RandomNumberGenerator rng;
	std::vector<glm::vec4> foliageLocations;
	foliageLocations.reserve(255);

	for (int j = 0; j < 255; j++)
	{
		float xoffset = rng.rand();
		float yoffset = rng.rand();

		glm::vec4 position = glm::vec4(
			xoffset,
			0.0f,
			yoffset,
			1.0f
		);

		foliageLocations.push_back(position);
	}

	glGenBuffers(1, &m_randomPatchSampleUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_randomPatchSampleUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * foliageLocations.size(), foliageLocations.data(), GL_DYNAMIC_DRAW);

	return true;
}

Resource<MeshCollection> FoliageSystem::getGrassBladeMesh()
{
	// TODO change, this is very specific to impl
	return m_grassBlade;
}

void FoliageSystem::setMeshLocations(const std::vector<glm::vec4>& locations)
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

	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_foliageChunksSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4), locations.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_visibleFoliageChunksSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_finalFoliageLocationsSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4) * 255, NULL, GL_DYNAMIC_DRAW);

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

unsigned int FoliageSystem::getInputSSBO() const
{
	return m_foliageChunksSSBO;
}

unsigned int FoliageSystem::getOutputSSBO() const
{
	return m_visibleFoliageChunksSSBO;
}

void FoliageSystem::setFrustum(Frustum& frustum)
{
	std::array<glm::vec4, 6> planes;
	planes[0] = glm::vec4(frustum.m_znear.m_normal, frustum.m_znear.m_distance);
	planes[1] = glm::vec4(frustum.m_zfar.m_normal, frustum.m_zfar.m_distance);
	planes[2] = glm::vec4(frustum.m_right.m_normal, frustum.m_right.m_distance);
	planes[3] = glm::vec4(frustum.m_left.m_normal, frustum.m_left.m_distance);
	planes[4] = glm::vec4(frustum.m_up.m_normal, frustum.m_up.m_distance);
	planes[5] = glm::vec4(frustum.m_down.m_normal, frustum.m_down.m_distance);

	glBindBuffer(GL_UNIFORM_BUFFER, m_frustumUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 6, planes.data(), GL_DYNAMIC_DRAW);
}

void FoliageSystem::drawFoliage(FoliageComponent& foliage)
{
	if (foliage.m_foliageSpreadMap.isEmpty() || count <= 0)
	{
		return;
	}

	auto graphics = Engine::get()->getSubSystem<Graphics>();
	// Perform frustum cull
	m_frustumCullComputeShader->use();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_foliageChunksSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_visibleFoliageChunksSSBO);

	m_frustumCullComputeShader->setUniformValue("positionsSize", count);

	{
		int texWidth = foliage.m_foliageSpreadMap->getWidth();
		int texHeight = foliage.m_foliageSpreadMap->getHeight();

		int instanceCount = texWidth * texHeight;

		{
			GLuint zero = 0;
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
			glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero); // reset counter to 0
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_frustumUBO);

		glDispatchCompute(ceil(instanceCount / 32.f), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
	GLuint result = ptr[0];
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	// fill blades on grass in each quad

	// populate grass chunks
	Resource<Shader>& populateGrassComputeShader = m_populateGrassComputeShader;
	populateGrassComputeShader->use();

	populateGrassComputeShader->setTextureInShader(foliage.m_foliageSpreadMap, "spreadMap", 0);
	populateGrassComputeShader->setUniformValue("textureWidth", foliage.m_foliageSpreadMap->getWidth());
	populateGrassComputeShader->setUniformValue("textureHeight", foliage.m_foliageSpreadMap->getHeight());
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_visibleFoliageChunksSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_finalFoliageLocationsSSBO);

	{
		GLuint zero = 0;
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero); // reset counter to 0
	}

	{
		int texWidth = foliage.m_foliageSpreadMap->getWidth();
		int texHeight = foliage.m_foliageSpreadMap->getHeight();

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_randomPatchSampleUBO);

		glDispatchCompute(ceil(texWidth / 32.f), ceil(texHeight / 32.f), 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	auto& foliageShader = m_foliageShader;
	foliageShader->use();
	foliageShader->setUniformValue("view", *graphics->view);
	foliageShader->setUniformValue("projection", *graphics->projection);
	foliageShader->setUniformValue("colorA", foliage.colorA);
	foliageShader->setUniformValue("colorB", foliage.colorB);
	//foliageShader->setUniformValue("viewDir", primaryCamera.front);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_finalFoliageLocationsSSBO);

	// create instance batch from foliage map

	//auto& grassBlade = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::GRASS_BLADE); 
	auto& grassBlade = m_grassBlade;
	auto vao = grassBlade->getPrimaryMesh()->getVAO();
	RenderCommand::drawInstanced(vao, result);
	//RenderCommand::draw(vao);
}
