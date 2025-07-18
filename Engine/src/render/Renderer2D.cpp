#include "render/Renderer2D.h"

#include "core/Logger.h"
#include <GL/glew.h>

#include "render/VertexArrayObject.h"
#include "camera/ICamera.h"
#include "geometry/Mesh.h"
#include "render/Graphics.h"

void Renderer2D::render()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();
	
	draw(*graphics->mesh->getVAO());
}

void Renderer2D::renderScene(Scene*)
{
}
