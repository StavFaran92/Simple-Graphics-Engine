#pragma once

#include "memory/ResourceRef.h"
#include "geometry/Model.h"


class Grid
{
public:
	static std::shared_ptr<Mesh> createMesh(int x, int y);
};