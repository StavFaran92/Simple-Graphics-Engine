#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "core/Core.h"
#include "systems/SubSystem.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/AssetAliases.h"

class EngineAPI SSAOSystem : public SubSystem
{
public:
	SSAOSystem();

	bool init();
	bool resize(int width, int height);

	void draw(TextureResourceRef positionVS, TextureResourceRef normalVS);

	TextureResourceRef getSSAOTexture() const { return m_blurColorBuffer; }

private:
	bool setup(int width, int height);

private:
	FrameBufferObject m_fbo;
	RenderBufferObject m_renderBuffer;
	TextureResourceRef m_colorBuffer = nullptr;
	ShaderResourceRef m_passShader;

	FrameBufferObject m_blurFBO;
	RenderBufferObject m_blurRenderBuffer;
	TextureResourceRef m_blurColorBuffer = nullptr;
	ShaderResourceRef m_blurPassShader;

	TextureResourceRef m_noiseTexture = nullptr;
	std::vector<glm::vec3> m_kernel;
};
