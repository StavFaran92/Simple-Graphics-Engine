#pragma once
#include "core/Core.h"

#include "geometry/MeshCollection.h"

class EngineAPI Sphere
{
public:
	static void createMesh(ResourceWrapper<MeshCollection>& meshCollection, float radius, int sectors, int stacks);
};
