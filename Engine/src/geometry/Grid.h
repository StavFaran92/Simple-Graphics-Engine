#pragma once

#include "Resource.h"
#include "MeshCollection.h"


class Grid
{
public:
	static void generateGrid(Resource<MeshCollection>& meshCollection,int x, int y);
};