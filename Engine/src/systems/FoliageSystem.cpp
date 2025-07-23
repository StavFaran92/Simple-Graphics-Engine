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
#include "systems/BuiltInMeshes.h"

FoliageSystem::FoliageSystem()
{
	Engine::get()->registerSubSystem<FoliageSystem>(this);
}

bool FoliageSystem::init()
{
	//m_foliageShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");
	m_foliageShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");
	m_foliageQuadShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageQuadShader.glsl");

	// At the moment i dont have infrastructure to import a mesh and alter its VAO in the same call.
	//auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/grass.obj"); // TODO use single blade model
	ModelImporter::ModelImportSettings settings;
	settings.isTransient = true;
	auto& modelInfo = Engine::get()->getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/scene.gltf", settings);

	//auto mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::QUAD);

	//Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_GRASS");
	//Quad::createMesh(meshCollection);
	m_grassBlade = modelInfo.mesh;

	glGenBuffers(1, &m_foliagePatchesSSBO);
	glGenBuffers(1, &m_visibleFoliagePatchesSSBO);
	glGenBuffers(1, &m_finalFoliageLocationsSSBO);

	m_frustumCullComputeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/FrustumCullComputeShader.glsl");
	m_populateGrassComputeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PopulateFoliageComputeShader.glsl");

	glGenBuffers(1, &m_atomicCounterBuffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterBuffer);

	glGenBuffers(1, &m_frustumUBO);

	RandomNumberGenerator rng;
	foliageRandomLocations.reserve(255);

	for (int j = 0; j < 255; j++)
	{
		float xoffset = rng.rand();
		float yoffset = rng.rand();

		glm::vec4 position = glm::vec4(
			xoffset,
			0.0f,
			yoffset,
			0.0f
		);

		foliageRandomLocations.push_back(position);
	}

	glGenBuffers(1, &m_randomPatchSampleUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_randomPatchSampleUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * foliageRandomLocations.size(), foliageRandomLocations.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_randomPatchSampleUBO);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			FoliagePatch patch;
			patch.pos = glm::vec3(i * 10, 0, j * 10);
			patch.width = 10;
			patch.height = 10;
			m_patches.push_back(patch);

		}

	}

	Texture::TextureImportSettings tSettings;
	tSettings.flip = true;
	grassTexture = Texture::importTexture2D(SGE_ROOT_DIR + "Resources/Engine/Textures/grass_v2.png", tSettings);

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

	//for (int i = 0; i < locations.size(); i++)
	//{
	//	FoliagePatch patch;
	//	patch.pos = glm::vec3(locations[i].x, locations[i].y, locations[i].z);
	//	patch.density = locations[i].w;
	//	m_patches.push_back(patch);

	//}

	//glGenBuffers(1, &m_patchOffsetUBO);
	//glBindBuffer(GL_UNIFORM_BUFFER, m_patchOffsetUBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * foliageLocations.size(), foliageLocations.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_randomPatchSampleUBO);

	
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_foliagePatchesSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4), locations.data(), GL_DYNAMIC_DRAW);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_visibleFoliagePatchesSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_finalFoliageLocationsSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, locations.size() * sizeof(glm::vec4) * 255, NULL, GL_DYNAMIC_DRAW);

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
	return m_foliagePatchesSSBO;
}

unsigned int FoliageSystem::getOutputSSBO() const
{
	return m_visibleFoliagePatchesSSBO;
}

void FoliageSystem::setFrustum(Frustum& frustum)
{
	m_frustum = frustum;

	//m_planes[0] = glm::vec4(frustum.m_znear.m_normal, frustum.m_znear.m_distance);
	//m_planes[1] = glm::vec4(frustum.m_zfar.m_normal, frustum.m_zfar.m_distance);
	//m_planes[2] = glm::vec4(frustum.m_right.m_normal, frustum.m_right.m_distance);
	//m_planes[3] = glm::vec4(frustum.m_left.m_normal, frustum.m_left.m_distance);
	//m_planes[4] = glm::vec4(frustum.m_up.m_normal, frustum.m_up.m_distance);
	//m_planes[5] = glm::vec4(frustum.m_down.m_normal, frustum.m_down.m_distance);

	//glBindBuffer(GL_UNIFORM_BUFFER, m_frustumUBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 6, planes.data(), GL_DYNAMIC_DRAW);
}

void FoliageSystem::setView(glm::vec3 pos, glm::vec3 front)
{
	m_camPos = pos;
	m_camFront = front;
}

float getSignedDistanceToPlane(glm::vec3 pos, const Plane& plane)
{
	return glm::dot(plane.m_normal, pos) - plane.m_distance;
}

bool isForwardOfPlane(glm::vec3 pos, const Plane& plane)
{
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	float r = 10 * abs(plane.m_normal.x) + 10 * abs(plane.m_normal.y) + 10 * abs(plane.m_normal.z);

	return -r <= getSignedDistanceToPlane(pos, plane);
}

bool isInFrustum(const Frustum& frustum, glm::vec3 pos)
{
	return isForwardOfPlane(pos, frustum.m_znear) &&
		isForwardOfPlane(pos, frustum.m_zfar) &&
		isForwardOfPlane(pos, frustum.m_right) &&
		isForwardOfPlane(pos, frustum.m_left) &&
		isForwardOfPlane(pos, frustum.m_up) &&
		isForwardOfPlane(pos, frustum.m_down);
}

void FoliageSystem::drawFoliage(FoliageComponent& foliage)
{
	if (foliage.m_foliageSpreadMap.isEmpty() || count <= 0)
	{
		return;
	}

	

	auto graphics = Engine::get()->getSubSystem<Graphics>();
	// Perform frustum cull
	//m_frustumCullComputeShader->use();

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_foliagePatchesSSBO);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_visibleFoliagePatchesSSBO);

	//m_frustumCullComputeShader->setUniformValue("positionsSize", count);

	//{
	//	int texWidth = foliage.m_foliageSpreadMap->getWidth();
	//	int texHeight = foliage.m_foliageSpreadMap->getHeight();

	//	int instanceCount = texWidth * texHeight;

	//	{
	//		GLuint zero = 0;
	//		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	//		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero); // reset counter to 0
	//	}

	//	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_frustumUBO);

	//	glDispatchCompute(ceil(instanceCount / 32.f), 1, 1);
	//	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//}

	// get visible patches back to CPU

	// sort the visible patches by distance from camera (front to back)

	// for each patch populate and draw

	// fill blades on grass in each quad

	// populate grass chunks
	//Resource<Shader>& populateGrassComputeShader = m_populateGrassComputeShader;
	//populateGrassComputeShader->use();

	////populateGrassComputeShader->setTextureInShader(foliage.m_foliageSpreadMap, "spreadMap", 0);
	//glBindImageTexture(
	//	0,                // binding = 0, must match `layout(binding = 0)`
	//	foliage.m_foliageSpreadMap->getID(),    // OpenGL texture ID
	//	0,                // mip level
	//	GL_FALSE,         // layered
	//	0,                // layer
	//	GL_READ_ONLY,     // or GL_WRITE_ONLY / GL_READ_WRITE
	//	GL_RGBA8           // must match internal format in texture creation
	//);

	//populateGrassComputeShader->setUniformValue("textureWidth", foliage.m_foliageSpreadMap->getWidth());
	//populateGrassComputeShader->setUniformValue("textureHeight", foliage.m_foliageSpreadMap->getHeight());
	//
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_visibleFoliagePatchesSSBO);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_finalFoliageLocationsSSBO);

	//{
	//	GLuint zero = 0;
	//	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	//	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero); // reset counter to 0
	//}

	//{
	//	int texWidth = foliage.m_foliageSpreadMap->getWidth();
	//	int texHeight = foliage.m_foliageSpreadMap->getHeight();

	//	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_randomPatchSampleUBO);

	//	glDispatchCompute(ceil(texWidth / 32.f), ceil(texHeight / 32.f), 1);
	//	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//}

	//glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
	//GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT);
	//GLuint result = ptr[0];
	//glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	std::vector<FoliagePatch> visiblePatches;
	for (int i = 0; i < m_patches.size(); i++)
	{
		if (isInFrustum(m_frustum, m_patches[i].pos))
		{
			visiblePatches.push_back(m_patches[i]);
		}
	}

	// I also need to sort back to front to achieve correct max LOD
	std::sort(visiblePatches.begin(), visiblePatches.end(), [this](const FoliagePatch& a, const FoliagePatch& b) {
		float da = glm::dot(a.pos - m_camPos, m_camFront);
		float db = glm::dot(b.pos - m_camPos, m_camFront);
		return da < db; // (front-to-back)
		});

	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);
	
	std::vector<FoliagePatch> patchesMaxLOD;
	for (int i = 0; i < visiblePatches.size(); i++)
	{

		float distance = glm::dot(visiblePatches[i].pos - m_camPos, m_camFront);

		
		

		if (distance < 100)
		{
			auto& foliageShader = m_foliageShader;
			foliageShader->use();
			foliageShader->setUniformValue("view", *graphics->view);
			foliageShader->setUniformValue("projection", *graphics->projection);
			foliageShader->setUniformValue("colorA", foliage.colorA);
			foliageShader->setUniformValue("colorB", foliage.colorB);
			foliageShader->setUniformValue("patchPosition", visiblePatches[i].pos);
			foliageShader->setUniformValue("patchSize", glm::vec2(visiblePatches[i].width, visiblePatches[i].height));
			foliageShader->setUniformValue("patchCount", glm::vec2(10, 10));
		
			auto& grassBlade = m_grassBlade;
			auto vao = grassBlade->getPrimaryMesh()->getVAO();
			RenderCommand::drawInstanced(vao, 255 * visiblePatches[i].width * visiblePatches[i].height);
		}
		else
		{
			patchesMaxLOD.insert(patchesMaxLOD.begin(), visiblePatches.begin() + i, visiblePatches.end());

			break;
		}
	}

	//glDisable(GL_DEPTH_TEST); 
	glDepthMask(GL_FALSE); //For max LOD grass i need to turn depth write off to get correct alpha blend
	auto& foliageShader = m_foliageQuadShader;
	foliageShader->use();
	foliageShader->setUniformValue("view", *graphics->view);
	foliageShader->setUniformValue("projection", *graphics->projection);
	foliageShader->setUniformValue("colorA", foliage.colorA);
	foliageShader->setUniformValue("colorB", foliage.colorB);
	

	for (int i = patchesMaxLOD.size()-1; i >= 0 ; i--)
	{
		foliageShader->setUniformValue("patchSize", glm::vec2(patchesMaxLOD[i].width, patchesMaxLOD[i].height));
		foliageShader->setUniformValue("patchCount", glm::vec2(10, 10));
		foliageShader->setTextureInShader(grassTexture, "grassTexture", 0);

		for (int j = 0; j < 100; j++)
		{
			glm::vec3 translation = glm::vec3(patchesMaxLOD[i].pos) + glm::vec3(foliageRandomLocations[j].x * 10, 0, foliageRandomLocations[j].z * 10);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
			//glm::mat4 rotationMatrix = glm::mat4_cast(localRotation);
			//glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), );
			//glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(visiblePatches[i].width, 1, visiblePatches[i].height));

			glm::mat4 model = translationMatrix/* * rotationMatrix*/ /** scaleMatrix*/;


			foliageShader->setUniformValue("model", model);



			auto& grassBlade = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::QUAD);
			auto vao = grassBlade->getPrimaryMesh()->getVAO();
			RenderCommand::draw(vao);
		}
	}
	//RenderCommand::draw(vao);
}
