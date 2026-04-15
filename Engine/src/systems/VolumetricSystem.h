#pragma once

#include "core/Core.h"
#include "systems/SubSystem.h"

#include "render/RenderCommand.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"

#include "texture/Texture.h"
#include "memory/ResourceWrapper.h"

class VolumeComponent;
class Scene;

class EngineAPI VolumetricSystem : public SubSystem
{
public:
	VolumetricSystem();

	static VolumetricSystem* get();

	bool init();

	void drawVolumetric(const VolumeComponent& volume, const glm::mat4& model);

private:
	std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	TextureResourceRef m_renderTargetTexture = TextureResourceRef::empty;

	ShaderResourceRef m_renderVolumeIntoSceneShader;
};