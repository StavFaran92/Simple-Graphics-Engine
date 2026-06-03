#include "render/SceneRenderBuffer.h"

void SceneBuffer::upload()
{
    // upload this frame's data
    m_ssbo.bind();
    m_ssbo.setData(m_objects.size() * sizeof(RenderData::ObjectData),
        m_objects.data());

    //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    //glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
    //    m_drawCommands.size() * sizeof(DrawCommand),
    //    m_drawCommands.data());
}

void SceneBuffer::bind(int slot)
{
	m_ssbo.setSlot(slot);
	m_ssbo.bind();
}