#pragma once
#include "core/Core.h"

#include "geometry/MeshGroup.h"

class EngineAPI Sphere
{
public:
	static std::shared_ptr<Mesh> createMesh(float radius, int sectors, int stacks);
};
