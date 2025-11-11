#pragma once

#include "render/IRenderer.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/ResourceWrapper.h"
#include "runtime/Entity.h"
#include "core/Window.h"
#include "core/Configurations.h"

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
	void reloadShaders();

	const FrameBufferObject& getGBuffer() const;

private:
	bool setupGBuffer(int width, int height);
	bool setupSSAO(int width, int height);

private:
	FrameBufferObject m_gBuffer;
	
	RenderBufferObject m_renderBuffer;
	
	ResourceWrapper<Texture> m_positionTexture = nullptr;
	ResourceWrapper<Texture> m_positionTextureVS = nullptr;
	ResourceWrapper<Texture> m_normalTexture = nullptr;
	ResourceWrapper<Texture> m_normalTextureVS = nullptr;
	ResourceWrapper<Texture> m_albedoTexture = nullptr;
	ResourceWrapper<Texture> m_MRATexture = nullptr;
	
	ResourceWrapper<MeshCollection> m_quad;
	ResourceWrapper<Shader> m_screenShader;
	
	ResourceWrapper<Shader> m_gBufferShader;
	ResourceWrapper<Shader> m_lightPassShader;

	// SSAO
	FrameBufferObject m_ssaoFBO;
	RenderBufferObject m_ssaoRenderBuffer;
	ResourceWrapper<Texture> m_ssaoNoiseTexture = nullptr;
	ResourceWrapper<Texture> m_ssaoColorBuffer = nullptr;
	ResourceWrapper<Shader> m_ssaoPassShader;
	std::vector<glm::vec3> m_ssaoKernel;
	
	FrameBufferObject m_ssaoBlurFBO;
	RenderBufferObject m_ssaoBlurRenderBuffer;
	ResourceWrapper<Texture> m_ssaoBlurColorBuffer = nullptr;
	ResourceWrapper<Shader> m_ssaoBlurPassShader;

};

