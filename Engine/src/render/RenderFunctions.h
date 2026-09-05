#pragma once

#include "core/Core.h"

class Mesh;
class Entity;
class Scene;
class GBuffer;

class EngineAPI RenderFunctions
{
public:
	static void drawGeometryToGBuffer(Scene* scene);
	static void drawInstancedGeometryToGBuffer(Scene* scene);
	static void drawLightPass(const GBuffer& gBuffer);
	static void drawForwardScene(Scene* scene);
	static void drawTransparentScene(Scene* scene);
	static void drawDebugData(Scene* scene);
	static void drawSceneUsingCustomShader(Scene* scene);

private:
	static bool prepareMeshForRender(Mesh* mesh, const Entity& entityHandler);
	static bool prepareEntityForRender(const Entity& entityHandler);
};
