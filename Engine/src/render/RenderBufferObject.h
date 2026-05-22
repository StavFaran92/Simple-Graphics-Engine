#pragma once
#include <cstdint>

#include "core/Core.h"

class EngineAPI RenderBufferObject
{
public:
	RenderBufferObject() = default;
	RenderBufferObject(int width, int height);

	uint32_t GetID() { return m_id; }

	void Bind();
	void Unbind();
private:
	uint32_t m_id;
};
