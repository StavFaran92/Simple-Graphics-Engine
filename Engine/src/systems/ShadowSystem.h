#pragma once

#include "core/Core.h"

#include "render/FrameBufferObject.h"
#include "render/IRenderer.h"
#include "runtime/Entity.h"
#include "memory/ResourceWrapper.h"

class Shader;
class Scene;
class Texture;
class Context;

class EngineAPI ShadowSystem
{
public:
	enum class ShadowAlgorithm
	{
		ShadowMapping,
		CSM
	};

	ShadowSystem(Context* context, Scene* scene);

	bool init();

	void renderToDepthMap();

	ResourceWrapper<Texture> getShadowMap() const;
	glm::mat4 getLightSpaceMat() const;
private:
	bool m_isInit = false;

	ShadowAlgorithm m_shadowAlgorithm = ShadowAlgorithm::ShadowMapping;

	FrameBufferObject m_fbo;
	Scene* m_scene = nullptr;
	ResourceWrapper<Shader> m_simpleDepthShader = nullptr;
	ResourceWrapper<Texture> m_depthMapTexture = nullptr;

	Context* m_context = nullptr;

	glm::mat4 m_lightSpaceMatrix;
};