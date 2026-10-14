#pragma once

#include "ThumbnailGenerator.h"

#include "render/FrameBufferObject.h"
#include "render/Shader.h"

// Box-filters a 2D texture down into the thumbnail, keeping its aspect ratio (letterboxed)
class TextureThumbnailGenerator : public IThumbnailGenerator
{
public:
	TextureThumbnailGenerator();

	bool generate(const AssetRecord& aInfo, const TextureResourceRef& target) override;

private:
	FrameBufferObject m_fbo;
	ShaderResourceRef m_shader;
};
