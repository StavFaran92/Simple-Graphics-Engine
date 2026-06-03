#pragma once

#include "render/IRenderer.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/SceneRenderBuffer.h"
#include "memory/AssetRef.h"
#include "runtime/Entity.h"
#include "core/Window.h"
#include "core/Configurations.h"
#include "memory/AssetAliases.h"

class Scene;

class IndirectRenderer
{
public:
	// Inherited via IRenderer
	bool init();
	void render();
	void renderScene(Scene* scene);
	void renderSceneUsingCustomShader(Scene* scene);
	void setUniforms(Shader* shader);

	void beginFrame();
	void endFrame();

	void resize(int w, int h);
	//void reloadShaders();

	const FrameBufferObject& getGBuffer() const;

	void addDrawCommand(const RenderData::DrawCommand& drawCommand) { m_drawCommands.push_back(drawCommand); }


private:
	bool setupGBuffer(int width, int height);

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

	SceneBuffer m_sceneBuffer;

	std::vector<RenderData::DrawCommand>  m_drawCommands;
	uint32_t m_indirectBuffer;
};