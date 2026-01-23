#pragma once

#include "memory/ResourceWrapper.h"
#include "geometry/MeshGroup.h"


class Grid
{
public:
	static void generateGrid(ResourceWrapper<MeshGroup>& meshCollection,int x, int y);
};