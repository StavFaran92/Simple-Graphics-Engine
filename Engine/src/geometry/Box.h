#pragma once

#include "core/Core.h"
#include "memory/ResourceWrapper.h"

#include "geometry/MeshCollection.h"

class EngineAPI Box
{
public:
	static void createMesh(ResourceWrapper<MeshCollection>& meshCollection);
};

