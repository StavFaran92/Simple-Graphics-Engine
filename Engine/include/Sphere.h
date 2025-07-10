#pragma once
#include "Core.h"

#include "MeshCollection.h"

class EngineAPI Sphere
{
public:
	static Resource<MeshCollection> createMesh(float radius, int sectors, int stacks);
};
