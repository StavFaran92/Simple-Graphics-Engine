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
	static void drawForwardScene(Scene* scene);
	static void drawTransparentScene(Scene* scene);
	static void drawDebugData(Scene* scene);
	static void drawSceneUsingCustomShader(Scene* scene);

private:
	static bool prepareMeshForRender(Mesh* mesh, const Entity& entityHandler);
	static bool prepareEntityForRender(const Entity& entityHandler);
};
