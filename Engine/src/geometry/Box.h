#pragma once

#include "core/Core.h"
#include "memory/Resource.h"

#include "geometry/MeshCollection.h"

class EngineAPI Box
{
public:
	static void createMesh(Resource<MeshCollection>& meshCollection);
};

