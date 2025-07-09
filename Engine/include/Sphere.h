#pragma once
#include "core/Core.h"

#include "geometry/Mesh.h"

class EngineAPI Sphere
{
public:
	static Resource<MeshCollection> createMesh(float radius, int sectors, int stacks);
};
