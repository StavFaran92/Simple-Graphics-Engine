#pragma once

#include "Resource.h"
#include "geometry/MeshCollection.h"


class Grid
{
public:
	static Resource<MeshCollection> generateGrid(int x, int y, bool isTransient = true);
};