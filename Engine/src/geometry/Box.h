#pragma once

#include "core/Core.h"
#include "memory/ResourceWrapper.h"

#include "geometry/MeshGroup.h"

class EngineAPI Box
{
public:
	static void createMesh(ResourceWrapper<MeshGroup>& meshCollection);
};

