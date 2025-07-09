#pragma once

#include "core/Core.h"
#include "memory/Resource.h"

class Mesh;

class EngineAPI Box
{
public:
	static Resource<MeshCollection> createMesh();
};

