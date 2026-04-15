#pragma once

#include "render/IRenderer.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/AssetRef.h"
#include "runtime/Entity.h"
#include "core/Window.h"
#include "core/Configurations.h"
#include "memory/AssetAliases.h"

class Scene;

class DeferredRenderer : public IRenderer
{
public:
	// Inherited via IRenderer
	bool init();
	void render();
	void renderScene(Scene* scene);
	void renderSceneUsingCustomShader(Scene* scene);
	void setUniforms(Shader* shader);

	void resize(int w, int h);
	//void reloadShaders();

	const FrameBufferObject& getGBuffer() const;

private:
	bool setupGBuffer(int width, int height);
	bool setupSSAO(int width, int height);

private:
	FrameBufferObject m_gBuffer;
	
	RenderBufferObject m_renderBuffer;
	
	// GBuffer
	TextureResourceRef m_positionTexture = nullptr;
	TextureResourceRef m_positionTextureVS = nullptr;
	TextureResourceRef m_normalTexture = nullptr;
	TextureResourceRef m_normalTextureVS = nullptr;
	TextureResourceRef m_albedoTexture = nullptr;
	TextureResourceRef m_MRATexture = nullptr;
	TextureResourceRef m_TangentTexture = nullptr;
	
	ModelResourceRef m_quad;
	ShaderResourceRef m_screenShader;

	// SSAO
	FrameBufferObject m_ssaoFBO;
	RenderBufferObject m_ssaoRenderBuffer;
	TextureResourceRef m_ssaoNoiseTexture = nullptr;
	TextureResourceRef m_ssaoColorBuffer = nullptr;
	ShaderResourceRef m_ssaoPassShader;
	std::vector<glm::vec3> m_ssaoKernel;
	
	FrameBufferObject m_ssaoBlurFBO;
	RenderBufferObject m_ssaoBlurRenderBuffer;
	TextureResourceRef m_ssaoBlurColorBuffer = nullptr;
	ShaderResourceRef m_ssaoBlurPassShader;

};

