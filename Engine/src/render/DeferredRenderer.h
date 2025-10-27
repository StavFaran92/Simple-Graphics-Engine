#pragma once

#include "render/RendererIntermediate.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/ResourceWrapper.h"
#include "runtime/Entity.h"
#include "core/Window.h"
#include "core/Configurations.h"

class FrameBufferObject;
class RenderBufferObject;

class DeferredRenderer : public RendererIntermediate
{
public:
	DeferredRenderer(Scene* scene);
	// Inherited via IRenderer
	bool init() override;
	void render() override;
	void renderScene(Scene* scene) override;
	void renderSceneUsingCustomShader(Scene* scene);
	void setUniforms(Shader* shader);

	void resize(int w, int h);

	const FrameBufferObject& getGBuffer() const;

private:
	bool setupGBuffer();
	bool setupSSAO();

private:
	FrameBufferObject m_gBuffer;
	
	RenderBufferObject m_renderBuffer{ Engine::get()->getWindow()->getWidth(),Engine::get()->getWindow()->getHeight() };
	
	ResourceWrapper<Texture> m_positionTexture = nullptr;
	ResourceWrapper<Texture> m_positionTextureVS = nullptr;
	ResourceWrapper<Texture> m_normalTexture = nullptr;
	ResourceWrapper<Texture> m_normalTextureVS = nullptr;
	ResourceWrapper<Texture> m_albedoTexture = nullptr;
	ResourceWrapper<Texture> m_MRATexture = nullptr;
	
	Scene* m_scene = nullptr;


	Entity m_quad;
	ResourceWrapper<Shader> m_screenShader;
	
	
	ResourceWrapper<Shader> m_gBufferShader;
	ResourceWrapper<Shader> m_lightPassShader;

	// SSAO
	FrameBufferObject m_ssaoFBO;
	RenderBufferObject m_ssaoRenderBuffer{ Engine::get()->getWindow()->getWidth(),Engine::get()->getWindow()->getHeight() };
	ResourceWrapper<Texture> m_ssaoNoiseTexture = nullptr;
	ResourceWrapper<Texture> m_ssaoColorBuffer = nullptr;
	ResourceWrapper<Shader> m_ssaoPassShader;
	std::vector<glm::vec3> m_ssaoKernel;
	
	FrameBufferObject m_ssaoBlurFBO;
	RenderBufferObject m_ssaoBlurRenderBuffer{ Engine::get()->getWindow()->getWidth(),Engine::get()->getWindow()->getHeight() };
	ResourceWrapper<Texture> m_ssaoBlurColorBuffer = nullptr;
	ResourceWrapper<Shader> m_ssaoBlurPassShader;

};

