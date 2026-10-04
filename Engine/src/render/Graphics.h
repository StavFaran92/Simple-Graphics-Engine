#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "memory/ResourceRef.h"
#include "runtime/Entity.h"
#include "systems/SubSystem.h"
#include "memory/AssetAliases.h"
#include "render/GBuffer.h"
#include "render/SSBO.h"
#include "render/DrawItem.h"

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

// Global directional-light shadow settings, consumed by ShadowSystem and the lighting shaders.
struct ShadowSettings
{
	bool enabled = true;

	// Shadow map resolution (square), changing it reallocates the depth texture
	int resolution = 2048;

	// Orthographic light projection
	float left = -10.0f;
	float right = 10.0f;
	float bottom = -10.0f;
	float top = 10.0f;
	float nearPlane = 1.0f;
	float farPlane = 200.0f;

	// Position the light view is rendered from (looks along the directional light's direction)
	glm::vec3 lightOrigin = { 0.0f, 100.0f, 0.0f };
};

class EngineAPI Graphics : public SubSystem
{
public:
	Graphics();
	
public:
	void reloadShaders();

	// Toggles vertical sync (swap interval) on the current GL context.
	void setVSync(bool enabled);
	bool isVSyncEnabled() const { return m_vsync; }
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
	ShadowSettings shadowSettings;

	GBuffer gBuffer;

	SSBO instancedModelBuffer;
	SSBO instancedAnimationBuffer;
	SSBO instancedInstanceDataBuffer;

private:
	bool m_vsync = false;
};