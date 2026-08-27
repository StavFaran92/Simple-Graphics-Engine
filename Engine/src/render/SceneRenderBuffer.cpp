#include "render/SceneRenderBuffer.h"

void SceneBuffer::upload()
{
    // upload this frame's data
    m_ssbo.bind();
    m_ssbo.setData(m_objects.size() * sizeof(RenderData::ObjectData), m_objects.data());

    m_materialssbo.bind();
    m_materialssbo.setData(m_materials.size() * sizeof(RenderData::MaterialData), m_materials.data());

    //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);
    //glBufferSubData(GL_DRAW_INDIRECT_BUFFER, 0,
    //    m_drawCommands.size() * sizeof(DrawCommand),
    //    m_drawCommands.data());
}

void SceneBuffer::clear()
{
    m_objects.clear();
    m_materials.clear();
}

void SceneBuffer::bind()
{
	m_ssbo.setSlot(2);
	m_ssbo.bind();

    m_materialssbo.setSlot(3);
    m_materialssbo.bind();
}