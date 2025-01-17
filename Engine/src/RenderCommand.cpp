#include "RenderCommand.h"
#include "Engine.h"
#include "System.h"

#include <GL/glew.h>

void RenderCommand::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderCommand::draw(const VertexArrayObject* vao)
{
	Engine::get()->getSubSystem<System>()->addTriangleCount(vao->GetIndexCount() / 3);

	vao->Bind();

	if (vao->GetIndexCount() == 0)
	{
		glDrawArrays(GL_TRIANGLES, 0, vao->GetVerticesCount());
	}
	else
	{
		glDrawElements(GL_TRIANGLES, vao->GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void RenderCommand::drawPatches(const VertexArrayObject* vao)
{
	vao->Bind();

	glDrawElements(GL_PATCHES, vao->GetIndexCount() , GL_UNSIGNED_INT, 0);
}

void RenderCommand::drawInstanced(const VertexArrayObject* vao, int count)
{
	vao->Bind();

	if (vao->GetIndexCount() == 0)
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, vao->GetVerticesCount(), count);
	}
	else
	{
		glDrawElementsInstanced(GL_TRIANGLES, vao->GetIndexCount(), GL_UNSIGNED_INT, 0, count);
	}
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
