#include "render/RendererIntermediate.h"

#include <GL/glew.h>

#include "core/Engine.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "render/ScreenQuad.h"
#include "render/Renderer2D.h"

void RendererIntermediate::draw(const VertexArrayObject& vao) const
{
	vao.Bind();

	if (vao.GetIndexCount() == 0)
	{
		glDrawArrays(m_drawType, 0, vao.GetVerticesCount());
	}
	else
	{
		glDrawElements(m_drawType, vao.GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void RendererIntermediate::clear() const
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}