#pragma once

#include "core/Core.h"
#include "memory/AssetAliases.h"

class Mesh;
class Entity;
class Scene;

class EngineAPI RenderFunctions
{
public:
	static void drawGeometryToGBuffer(Scene* scene);
	static void drawLightPass();

private:
	static bool prepareMeshForRender(Mesh* mesh, const Entity& entityHandler);
	static bool prepareEntityForRender(const Entity& entityHandler);
};
