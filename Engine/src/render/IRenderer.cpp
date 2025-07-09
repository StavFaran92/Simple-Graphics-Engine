#include "IRenderer.h"

#include "Logger.h"
#include <GL/glew.h>

void IRenderer::SetDrawType(DrawType drawType)
{
	switch (drawType)
	{
	case DrawType::Lines:
		m_drawType = GL_LINES;
		break;
	case DrawType::Triangles:
		m_drawType = GL_TRIANGLES;
		break;
	default:
		logError("Illegal draw type specified.");
	}
}