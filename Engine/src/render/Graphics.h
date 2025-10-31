#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "memory/ResourceWrapper.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"

class Scene;
class Context;
class IRenderer;
class Entity;
class Mesh;
class Shader;
class Material;
class Texture;
class Frustum;
class RenderView;

enum class RenderMode
{
	SHADED = 0,
	WIREFRAME = 1
};

class EngineAPI Graphics : public SubSystem
{
public:
	Graphics();
	
public:
	void reloadDefferedRendererShaders();
public:
	Scene* scene = nullptr;
	Context* context = nullptr;
	glm::vec3 cameraPos;

	std::vector<Entity> entityGroup;

	Entity* entity = nullptr;
	Mesh* mesh = nullptr;
	ResourceWrapper<Shader> shader = nullptr;
	Material* material = nullptr;

	// MVP
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	ResourceWrapper<Texture> irradianceMap = nullptr;
	ResourceWrapper<Texture> prefilterEnvMap = nullptr;
	ResourceWrapper<Texture> brdfLUT = nullptr;
	ResourceWrapper<Texture> shadowMap = nullptr;
	glm::mat4 lightSpaceMatrix;
	Frustum* frustum = nullptr;
	std::shared_ptr<RenderView> renderView;

	RenderMode renderMode = RenderMode::SHADED;

	bool useSSAO = true;
};