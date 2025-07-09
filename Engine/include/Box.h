#pragma once

#include "Core.h"
#include "Resource.h"

class Mesh;

class EngineAPI Box
{
public:
	static Resource<MeshCollection> createMesh();
};

