#pragma once

#include "memory/Resource.h"
#include "geometry/MeshCollection.h"
#include "component/FoliageComponent.h"

struct FoliagePatch
{
	glm::vec3 pos{};
	float density = 0;
	int idx = 0;
	int idy = 0;
	int width = 10;
	int height = 10;
	int LOD = 0;
};

class FoliageSystem
{
public:
	FoliageSystem();
	bool init();

	Resource<MeshCollection> getGrassBladeMesh();
	void setMeshLocations(const std::vector<glm::vec4>& locations);
	int getCount() const;
	Resource<Shader>& getFoliageShader();
	unsigned int getInputSSBO() const;
	unsigned int getOutputSSBO() const;

	void setFrustum(Frustum& frustum);
	void setView(glm::vec3 pos, glm::vec3 front);

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

	std::vector<FoliagePatch> m_patches;
	glm::vec3 m_camPos;
	glm::vec3 m_camFront;

	Frustum m_frustum;

	std::vector<glm::mat4>foliageRandomTransforms;
	std::vector<glm::vec3>foliageRandomLocations;
	Resource<Texture>grassTexture;
};