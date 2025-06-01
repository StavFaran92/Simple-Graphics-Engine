#pragma once

#include "Resource.h"
#include "MeshCollection.h"


class Grid
{
public:
	static Resource<MeshCollection> generateGrid(int x, int y, bool isTransient = true);
};