#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "memory/ResourceWrapper.h"
#include "runtime/Entity.h"

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

class Graphics
{
public:
	Graphics();
	
public:
	Scene* scene = nullptr;
	Context* context = nullptr;
	IRenderer* renderer = nullptr;
	glm::vec3 cameraPos;

	std::vector<Entity> entityGroup;

	Entity* entity = nullptr;
	Mesh* mesh = nullptr;
	ResourceWrapper<Shader> shader = nullptr;
	Material* material = nullptr;

	// MVP
	glm::mat4* model = nullptr;
	glm::mat4* view = nullptr;
	glm::mat4* projection = nullptr;

	ResourceWrapper<Texture> irradianceMap = nullptr;
	ResourceWrapper<Texture> prefilterEnvMap = nullptr;
	ResourceWrapper<Texture> brdfLUT = nullptr;
	ResourceWrapper<Texture> shadowMap = nullptr;
	glm::mat4 lightSpaceMatrix;
	Frustum* frustum = nullptr;
	std::shared_ptr<RenderView> renderView;

	RenderMode renderMode = RenderMode::SHADED;
};