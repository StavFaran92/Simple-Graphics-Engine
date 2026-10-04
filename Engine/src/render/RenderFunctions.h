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
	static void drawGeometryPass(Scene* scene, const std::vector<DrawItem>& drawItems);
	//static void drawInstancedGeometryToGBuffer(Scene* scene);
	// Draw items based instanced G-Buffer pass, batches and bone transforms are built in Scene::generateDrawItems
	static void drawInstancedGeometryToGBuffer(Scene* scene, const std::vector<DrawItem>& drawItems);
	static void drawLightPass(const GBuffer& gBuffer);
	static void drawTransparentScene(Scene* scene, const std::vector<DrawItem>& drawItems);
	static void drawDebugData(Scene* scene);
	static void drawSceneUsingCustomShader(Scene* scene);

private:
	static bool prepareMeshForRender(Mesh* mesh, const Entity& entityHandler);
	static bool prepareEntityForRender(const Entity& entityHandler);

};
