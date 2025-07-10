#pragma once

#include "Core.h"

#include "render/FrameBufferObject.h"
#include "render/IRenderer.h"
#include "Entity.h"

class Shader;
class Scene;
class ScreenBufferDisplay;
class TextureHandler;

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

	Resource<Texture> getShadowMap() const;
	glm::mat4 getLightSpaceMat() const;
private:
	bool m_isInit = false;

	ShadowAlgorithm m_shadowAlgorithm = ShadowAlgorithm::ShadowMapping;

	FrameBufferObject m_fbo;
	Scene* m_scene = nullptr;
	Resource<Shader> m_simpleDepthShader = nullptr;
	Resource<Texture> m_depthMapTexture = nullptr;

	std::shared_ptr<ScreenBufferDisplay> m_bufferDisplay;

	Context* m_context = nullptr;

	glm::mat4 m_lightSpaceMatrix;
};