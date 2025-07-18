#pragma once

#include "memory/Resource.h"
#include "geometry/MeshCollection.h"


class Grid
{
public:
	static void generateGrid(Resource<MeshCollection>& meshCollection,int x, int y);
};