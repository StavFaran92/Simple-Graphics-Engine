#pragma once

#include "core/Core.h"
#include "memory/ResourceWrapper.h"

#include "geometry/Model.h"

class EngineAPI Box
{
public:
	static void createMesh(ResourceWrapper<Model>& meshCollection);
};

