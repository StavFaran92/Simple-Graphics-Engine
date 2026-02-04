#pragma once
#include "core/Core.h"

#include "geometry/Model.h"

class EngineAPI Sphere
{
public:
	static void createMesh(ResourceWrapper<Model>& meshCollection, float radius, int sectors, int stacks);
};
