#pragma once
#include "core/Core.h"

#include "geometry/MeshGroup.h"

class EngineAPI Sphere
{
public:
	static void createMesh(ResourceWrapper<MeshGroup>& meshCollection, float radius, int sectors, int stacks);
};
