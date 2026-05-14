#pragma once
#include "core/Core.h"

#include "geometry/Model.h"

class EngineAPI Cylinder
{
public:
	static std::shared_ptr<Mesh> createMesh(float height, float radius, int sectors);
};
