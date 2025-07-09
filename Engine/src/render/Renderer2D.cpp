#include "Renderer2D.h"

#include "Logger.h"
#include <GL/glew.h>

#include "VertexArrayObject.h"
#include "ICamera.h"
#include "Mesh.h"
#include "Graphics.h"

void Renderer2D::render()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();
	
	draw(*graphics->mesh->getVAO());
}

void Renderer2D::renderScene(Scene*)
{
}
