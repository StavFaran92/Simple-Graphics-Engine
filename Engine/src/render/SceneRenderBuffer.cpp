#include "render/SceneRenderBuffer.h"

void SceneBuffer::upload()
{
}

void SceneBuffer::bind(int slot)
{
	m_ssbo.setSlot(slot);
	m_ssbo.bind();
}