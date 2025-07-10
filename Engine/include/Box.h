#pragma once

#include "Core.h"
#include "Resource.h"

#include "MeshCollection.h"

class EngineAPI Box
{
public:
	static Resource<MeshCollection> createMesh();
};

