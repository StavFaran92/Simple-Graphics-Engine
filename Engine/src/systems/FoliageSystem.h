#pragma once

#include "memory/ResourceWrapper.h"
#include "geometry/MeshCollection.h"
#include "component/FoliageField.h"
#include "systems/SubSystem.h"

class FoliageSystem : public SubSystem
{
public:
	FoliageSystem();
	bool init();

	void setFrustum(Frustum& frustum);
	void setView(glm::vec3 pos, glm::vec3 front);

	glm::vec3 getRandomLocation(int instanceID) const;

	void drawFoliage(FoliageField& foliage);

private:
	ResourceWrapper<MeshCollection> m_grassBlade;
	int count = 0;
	ResourceWrapper<Shader> m_foliageQuadShader;

	unsigned int m_foliagePatchesSSBO;
	unsigned int m_visibleFoliagePatchesSSBO;
	unsigned int m_finalFoliageLocationsSSBO;

	ResourceWrapper<Shader> m_sampleComputeShader;
	ResourceWrapper<Shader> m_frustumCullComputeShader;
	ResourceWrapper<Shader> m_populateGrassComputeShader;
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
	ResourceWrapper<Texture>grassTexture;
	ResourceWrapper<Texture>windNoise;
	ResourceWrapper<Texture>noiseTexture;

	int maxFoliageViewDistance = 100;
	int minFoliageQuadViewDistance = 50;
};