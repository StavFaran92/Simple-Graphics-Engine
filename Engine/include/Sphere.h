#pragma once
#include "Core.h"

#include "MeshCollection.h"

class EngineAPI Sphere
{
public:
	static void createMesh(Resource<MeshCollection>& meshCollection, float radius, int sectors, int stacks);
};
