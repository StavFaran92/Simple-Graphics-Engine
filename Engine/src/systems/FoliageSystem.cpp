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

#include "systems/TimeManager.h"
#include "component/Terrain.h"
#include <glm/ext.hpp>

FoliageSystem::FoliageSystem()
{
	Engine::get()->registerSubSystem<FoliageSystem>(this);
}

bool FoliageSystem::init()
{
	m_foliageShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageShader.glsl");
	m_foliageQuadShader = Shader::load(SGE_ROOT_DIR + "Resources/Engine/Shaders/FoliageQuadShader.glsl");

	ModelImportSettings settings;
	settings.isEngineOwned = true;
	m_grassBlade = MeshCollection::load(SGE_ROOT_DIR + "Resources/Engine/Meshes/grass_blade.fbx", settings);

	glGenBuffers(1, &m_frustumUBO);

	RandomNumberGenerator rng;
	foliageRandomLocations.reserve(1000);

	for (int j = 0; j < 1000; j++)
	{
		float xoffset = rng.rand();
		float yoffset = rng.rand();
		float scaleFactor = rng.rand() + .5;
		float yawRotationFactor = rng.rand() * 2 * Constants::PI; //should probably use noise here

		glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(xoffset, 0.0f, yoffset));
		glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1, scaleFactor, 1));
		glm::mat4 rot = glm::mat4(1.0);
		//glm::mat4 rot = glm::rotate(glm::mat4(1.0), yawRotationFactor, glm::vec3(0,1,0));

		glm::mat4 trans = translate * rot * scale;

		//foliageRandomTransforms.push_back(trans);
		foliageRandomLocations.push_back(glm::vec3(xoffset, 0.0f, yoffset));
	}

	//glGenBuffers(1, &m_randomPatchSampleUBO);
	//glBindBuffer(GL_UNIFORM_BUFFER, m_randomPatchSampleUBO);
	//glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * foliageRandomLocations.size(), foliageRandomLocations.data(), GL_DYNAMIC_DRAW);
	//glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_randomPatchSampleUBO);

	{
		Texture::TextureAssetDescriptor tSettings;
		tSettings.flip = true;
		grassTexture = Texture::load(SGE_ROOT_DIR + "Resources/Engine/Textures/grass_v2.png", tSettings);
	}

	{
		Texture::TextureAssetDescriptor noiseSettings;
		noiseSettings.params[GL_TEXTURE_WRAP_S] = GL_MIRRORED_REPEAT;
		noiseSettings.params[GL_TEXTURE_WRAP_T] = GL_MIRRORED_REPEAT;
		noiseSettings.params[GL_TEXTURE_MIN_FILTER] = GL_LINEAR;
		noiseSettings.params[GL_TEXTURE_MAG_FILTER] = GL_LINEAR;
		windNoise = Texture::load(SGE_ROOT_DIR + "Resources/Engine/Textures/wind_noise.png", noiseSettings);
	}

	{
		Texture::TextureAssetDescriptor noiseSettings;
		noiseSettings.params[GL_TEXTURE_WRAP_S] = GL_MIRRORED_REPEAT;
		noiseSettings.params[GL_TEXTURE_WRAP_T] = GL_MIRRORED_REPEAT;
		noiseSettings.params[GL_TEXTURE_MIN_FILTER] = GL_LINEAR;
		noiseSettings.params[GL_TEXTURE_MAG_FILTER] = GL_LINEAR;
		noiseTexture = Texture::load(SGE_ROOT_DIR + "Resources/Engine/Textures/noiseTexture.png", noiseSettings);
	}

	return true;
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

glm::vec3 FoliageSystem::getRandomLocation(int instanceID) const
{
	size_t size = foliageRandomLocations.size();
	return foliageRandomLocations[instanceID % size];
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

void FoliageSystem::drawFoliage(FoliageField& foliage)
{
	if (foliage.m_foliageSpreadMap.size() == 0)
	{
		return;
	}

	auto graphics = Engine::get()->getSubSystem<Graphics>();

	std::vector<std::shared_ptr<FoliagePatch>> visiblePatches;
	const auto& patches = foliage.getPatches();
	visiblePatches.reserve(patches.size());
	for (int i = 0; i < patches.size(); i++)
	{
		if (isInFrustum(m_frustum, patches[i]->pos))
		{
			visiblePatches.push_back(patches[i]);
		}
	}

	// I also need to sort back to front to achieve correct max LOD
	std::sort(visiblePatches.begin(), visiblePatches.end(), [this](const std::shared_ptr<FoliagePatch>& a, const std::shared_ptr < FoliagePatch>& b) {
		float da = glm::dot(glm::vec2(a->pos.x, a->pos.z) - glm::vec2(m_camPos.x, m_camPos.z), glm::vec2(m_camFront.x, m_camFront.z));
		float db = glm::dot(glm::vec2(b->pos.x, b->pos.z) - glm::vec2(m_camPos.x, m_camPos.z), glm::vec2(m_camFront.x, m_camFront.z));
		return da < db; // (front-to-back)
		});

	//glEnable(GL_DEPTH_TEST); 
	//glDepthMask(GL_TRUE);
	//glEnable(GL_CULL_FACE);

	auto& foliageShader = m_foliageShader;
	foliageShader->use();
	foliageShader->setUniformValue("view", graphics->view);
	foliageShader->setUniformValue("projection", graphics->projection);
	foliageShader->setUniformValue("colorA", foliage.colorA);
	foliageShader->setUniformValue("colorB", foliage.colorB);
	foliageShader->setTextureInShader(windNoise, "windNoise", 0);
	foliageShader->setTextureInShader(noiseTexture, "noiseTexture", 2);
	foliageShader->setUniformValue("time", (float)Engine::get()->getTimeManager()->getElapsedTime(TimeManager::Duration::MilliSeconds) / 1000);

	float phi = -atan2f(m_camFront.z, m_camFront.x);
	glm::quat q(glm::vec3(0.f, phi + Constants::PI / 2, 0.f));
	glm::mat4 rotationMatrix = glm::mat4_cast(q);
	foliageShader->setUniformValue("rotation", rotationMatrix);

	auto scale = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
	foliageShader->setUniformValue("scale", scale);

	foliageShader->setUniformValue("viewDir", m_camFront);
	
	std::vector<std::shared_ptr<FoliagePatch>> patchesMaxLOD;
	for (int i = 0; i < visiblePatches.size(); i++)
	{

		float distance = glm::dot(visiblePatches[i]->pos - m_camPos, m_camFront);

		if (distance < maxFoliageViewDistance)
		{
			foliageShader->setUniformValue("patchPosition", visiblePatches[i]->pos);

			//auto& grassBlade = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::SPHERE);
			auto& grassBlade = m_grassBlade;
			auto vao = grassBlade->getPrimaryMesh()->getVAO();

			float density = std::min(1.0f, std::max(0.f, maxFoliageViewDistance - distance) / maxFoliageViewDistance);
			int instanceCount = visiblePatches[i]->instanceCount * density;

			//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * instanceCount, visiblePatches[i]->instancesData.data());
			RenderCommand::drawInstanced(vao, instanceCount);
		}

		if(distance > minFoliageQuadViewDistance)
		{
			if (distance > maxFoliageViewDistance)
			{
				patchesMaxLOD.insert(patchesMaxLOD.begin(), visiblePatches.begin() + i, visiblePatches.end());
				break;
			}
			else
			{
				patchesMaxLOD.push_back(*(visiblePatches.begin() + i));
			}
		}

		
	}

	//glDisable(GL_DEPTH_TEST); 
	//glDepthMask(GL_FALSE); //For max LOD grass i need to turn depth write off to get correct alpha blend
	//auto& foliageMAXLODShader = m_foliageQuadShader;
	//foliageMAXLODShader->use();
	//foliageMAXLODShader->setUniformValue("view", *graphics->view);
	//foliageMAXLODShader->setUniformValue("projection", *graphics->projection);
	//foliageMAXLODShader->setUniformValue("colorA", foliage.colorA);
	//foliageMAXLODShader->setUniformValue("colorB", foliage.colorB);
	//foliageMAXLODShader->setUniformValue("patchSize", glm::vec2(foliage.patchWidth, foliage.patchHeight));
	//foliageMAXLODShader->setUniformValue("patchCount", glm::vec2(10, 10));
	//foliageMAXLODShader->setTextureInShader(grassTexture, "grassTexture", 0);
	//foliageMAXLODShader->setUniformValue("rotation", rotationMatrix);
	//foliageMAXLODShader->setUniformValue("offset", glm::vec4(0, .5, 0, 0));
	//auto& grassBlade = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::QUAD);
	//auto vao = grassBlade->getPrimaryMesh()->getVAO();
	//
	//for (int i = patchesMaxLOD.size()-1; i >= 0 ; i--)
	//{
	//	float density = 1.f;
	//	int instanceCount = 1000 * density;

	//	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * instanceCount, patchesMaxLOD[i]->instancesData.data());
	//	RenderCommand::drawInstanced(vao, instanceCount);
	//}
}
