#pragma once

#include "memory/ResourceWrapper.h"
#include "geometry/MeshGroup.h"


class Grid
{
public:
	static std::shared_ptr<Mesh> createMesh(int x, int y);
};