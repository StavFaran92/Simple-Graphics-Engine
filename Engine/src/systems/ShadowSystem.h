#pragma once

#include "core/Core.h"

#include "render/FrameBufferObject.h"
#include "memory/AssetAliases.h"

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

	ShadowSystem();

	bool init();

	void renderToDepthMap();

	TextureResourceRef getShadowMap() const;
	glm::mat4 getLightSpaceMat() const;
private:
	bool m_isInit = false;

	ShadowAlgorithm m_shadowAlgorithm = ShadowAlgorithm::ShadowMapping;

	FrameBufferObject m_fbo;
	//Scene* m_scene = nullptr;
	ShaderResourceRef m_simpleDepthShader = nullptr;
	TextureResourceRef m_depthMapTexture = nullptr;

	//Context* m_context = nullptr;

	glm::mat4 m_lightSpaceMatrix;
};