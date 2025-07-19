#pragma once

#include "memory/Resource.h"
#include "geometry/MeshCollection.h"
#include "component/FoliageComponent.h"

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

	void drawFoliage(FoliageComponent& foliage);

private:
	Resource<MeshCollection> m_grassBlade;
	int count = 0;
	Resource<Shader> m_foliageShader;

	unsigned int m_foliageChunksSSBO;
	unsigned int m_visibleFoliageChunksSSBO;
	unsigned int m_finalFoliageLocationsSSBO;

	Resource<Shader> m_sampleComputeShader;
	Resource<Shader> m_frustumCullComputeShader;
	Resource<Shader> m_populateGrassComputeShader;
	unsigned int m_atomicCounterBuffer;
	unsigned int m_frustumUBO;
	unsigned int m_randomPatchSampleUBO;
};