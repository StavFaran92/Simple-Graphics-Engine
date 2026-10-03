#pragma once

#include <vector>

#include "core/Core.h"

class Mesh;
class Entity;
class Scene;
class GBuffer;
struct DrawItem;

class EngineAPI RenderFunctions
{
public:
	// Draw items based frame rendering - only the main (G-Buffer) pass for now
	static void renderFrame(Scene* scene);

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

	static void drawMainPass(Scene* scene, const std::vector<DrawItem>& drawItems);
};
