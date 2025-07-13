#pragma once

#include "Resource.h"
#include "MeshCollection.h"

class FoliageSystem
{
public:
	FoliageSystem();
	bool init();

	Resource<MeshCollection> getGrassBladeMesh();
	void setMeshLocations(const std::vector<glm::vec3>& locations);
	int getCount() const;
	Resource<Shader>& getFoliageShader();

private:
	Resource<MeshCollection> m_grassBlade;
	int count = 0;
	Resource<Shader> m_foliageShader;
};