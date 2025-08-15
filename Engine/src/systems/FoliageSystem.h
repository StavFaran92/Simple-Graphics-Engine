#pragma once

#include "memory/Resource.h"
#include "geometry/MeshCollection.h"
#include "component/FoliageComponent.h"

class FoliageSystem
{
public:
	FoliageSystem();
	bool init();

	void setFrustum(Frustum& frustum);
	void setView(glm::vec3 pos, glm::vec3 front);

	glm::vec3 getRandomLocation(int instanceID) const;

	void drawFoliage(FoliageComponent& foliage);

private:
	Resource<MeshCollection> m_grassBlade;
	int count = 0;
	Resource<Shader> m_foliageShader;
	Resource<Shader> m_foliageQuadShader;

	unsigned int m_foliagePatchesSSBO;
	unsigned int m_visibleFoliagePatchesSSBO;
	unsigned int m_finalFoliageLocationsSSBO;

	Resource<Shader> m_sampleComputeShader;
	Resource<Shader> m_frustumCullComputeShader;
	Resource<Shader> m_populateGrassComputeShader;
	unsigned int m_atomicCounterBuffer;
	unsigned int m_frustumUBO;
	unsigned int m_randomPatchSampleUBO;
	unsigned int m_patchOffsetUBO;
	unsigned int m_patchInstanceDataSSBO;

	//std::vector<FoliagePatch> m_patches;
	glm::vec3 m_camPos;
	glm::vec3 m_camFront;

	Frustum m_frustum;

	//std::vector<glm::mat4>foliageRandomTransforms;
	std::vector<glm::vec3>foliageRandomLocations;
	Resource<Texture>grassTexture;
	Resource<Texture>windNoise;
	Resource<Texture>noiseTexture;

	int maxFoliageViewDistance = 100;
	int minFoliageQuadViewDistance = 50;
};