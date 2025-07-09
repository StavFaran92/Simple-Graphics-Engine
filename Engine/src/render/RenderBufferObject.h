#pragma once
#include <cstdint>

class RenderBufferObject
{
public:
	RenderBufferObject(int width, int height);

	uint32_t GetID() { return m_id; }

	void Bind();
	void Unbind();
private:
	uint32_t m_id;
};
