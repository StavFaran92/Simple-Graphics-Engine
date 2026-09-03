#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "memory/ResourceRef.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"
#include "memory/AssetAliases.h"
#include "render/GBuffer.h"

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
	void reloadShaders();
public:
	Scene* scene = nullptr;
	Context* context = nullptr;
	glm::vec3 cameraPos;

	std::vector<Entity> entityGroup;

	Entity entity = Entity::EmptyEntity;
	Mesh* mesh = nullptr;
	ShaderResourceRef shader = nullptr;
	MaterialResourceRef material = nullptr;

	// MVP
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	TextureResourceRef irradianceMap = nullptr;
	TextureResourceRef prefilterEnvMap = nullptr;
	TextureResourceRef brdfLUT = nullptr;
	TextureResourceRef shadowMap = nullptr;
	TextureResourceRef ssaoTexture = nullptr;
	glm::mat4 lightSpaceMatrix;
	Frustum* frustum = nullptr;
	std::shared_ptr<RenderView> renderView;

	RenderMode renderMode = RenderMode::SHADED;

	bool useSSAO = true;

	GBuffer gBuffer;
};