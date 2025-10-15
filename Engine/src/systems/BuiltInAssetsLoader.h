#pragma once

#include <map>
#include "memory/ResourceWrapper.h"

class BuiltInAssetsLoader
{
public:
	static void loadAssets();
private:
	static void loadTextures();
	static void loadMaterials();
	static void loadMeshes();
};