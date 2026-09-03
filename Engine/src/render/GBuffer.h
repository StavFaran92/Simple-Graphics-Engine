#pragma once

#include <cstdint>

#include "core/Core.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/AssetAliases.h"

class EngineAPI GBuffer
{
public:
	enum class Attachment
	{
		Position,
		Normal,
		Albedo,
		MRA,
		PositionVS,
		NormalVS,
		Tangent,

		Count
	};

	bool setup(int width, int height);

	bool isReady() const { return m_isReady; }

	void bind();
	void unbind();

	uint32_t getID() const;

	TextureResourceRef getTexture(Attachment attachment) const;
	void setTexture(Attachment attachment, TextureResourceRef texture);

private:
	FrameBufferObject m_fbo;
	RenderBufferObject m_renderBuffer;

	TextureResourceRef m_textures[(size_t)Attachment::Count];

	bool m_isReady = false;
};
