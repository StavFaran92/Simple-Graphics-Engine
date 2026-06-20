#include "render/RenderCommand.h"
#include "core/Engine.h"
#include "core/System.h"
#include "core/Window.h"
#include "render/Graphics.h"
#include "render/RenderView.h"
#include "render/VertexArrayObject.h"

#include <GL/glew.h>

void RenderCommand::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::draw(const VertexArrayObject& vao)
{
	Engine::get()->getSubSystem<System>()->addTriangleCount(vao.GetIndexCount() / 3); // todo remove

	vao.Bind();

	if (vao.GetIndexCount() == 0)
	{
		glDrawArrays(GL_TRIANGLES, 0, vao.GetVerticesCount());
	}
	else
	{
		glDrawElements(GL_TRIANGLES, vao.GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void RenderCommand::drawPatches(const VertexArrayObject& vao)
{
	vao.Bind();

	glDrawElements(GL_PATCHES, vao.GetIndexCount() , GL_UNSIGNED_INT, 0);
}

void RenderCommand::drawInstanced(const VertexArrayObject& vao, int count)
{
	vao.Bind();

	if (vao.GetIndexCount() == 0)
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, vao.GetVerticesCount(), count);
	}
	else
	{
		glDrawElementsInstanced(GL_TRIANGLES, vao.GetIndexCount(), GL_UNSIGNED_INT, 0, count);
	}
}

void RenderCommand::setViewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}

void RenderCommand::copyFrameBufferData(unsigned int src, unsigned int dst, int bufferBit)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Bind G-Buffer as src
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src);

	// Bind default buffer as dest
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);

    auto renderView = graphics->renderView;

    assert(renderView);

    auto& viewport = renderView->getViewport();

    int destW = viewport.w;
    int destH = viewport.h;

    // If copying to the default framebuffer, use the window dimensions as
    // the destination size to ensure the final image covers the window.
    if (dst == 0)
    {
        auto window = Engine::get()->getWindow();
        if (window)
        {
            destW = window->getWidth();
            destH = window->getHeight();
        }
    }

    // Copy src to dest
    glBlitFramebuffer(0, 0, viewport.w, viewport.h,
            0, 0, destW, destH,
            bufferBit, GL_NEAREST);


    glBindFramebuffer(GL_FRAMEBUFFER, dst);
}

//void RenderCommand::drawIndexed(const VertexArrayObject* vao)
//{
//	vao->Bind();
//
//	if (vao->GetIndexCount() == 0)
//	{
//		glDrawArrays(GL_TRIANGLES, 0, vao->GetVerticesCount());
//	}
//	else
//	{
//		glDrawElements(GL_TRIANGLES, vao->GetIndexCount(), GL_UNSIGNED_INT, 0);
//	}
//}
