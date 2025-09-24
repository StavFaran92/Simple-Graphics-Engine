#pragma once

#include "memory/ResourceWrapper.h"
#include "geometry/MeshCollection.h"


class Grid
{
public:
	static void generateGrid(ResourceWrapper<MeshCollection>& meshCollection,int x, int y);
};