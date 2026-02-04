#pragma once

#include "memory/ResourceWrapper.h"
#include "geometry/Model.h"


class Grid
{
public:
	static void generateGrid(ResourceWrapper<Model>& meshCollection,int x, int y);
};