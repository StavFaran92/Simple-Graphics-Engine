#include "render/ScreenBufferDisplay.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/Renderer2D.h"
#include "render/ScreenQuad.h"
#include "Engine.h"
#include "Window.h"
#include "Logger.h"
#include "Resource.h"
#include "Scene.h"
#include "Entity.h"
#include "Component.h"
#include "Texture.h"
#include "MeshCollection.h"
#include "render/Renderer.h"
#include "render/Graphics.h"

#include "GL/glew.h"

ScreenBufferDisplay::ScreenBufferDisplay(Scene* scene)
{
	m_scene = scene;
}

bool ScreenBufferDisplay::init(int windowWidth, int windowHeight)
{
	// Generate screen quad
	m_quad = ScreenQuad::GenerateScreenQuad(&m_scene->getRegistry());
	
	// Generate screen shader
	m_screenShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/ScreenBufferDisplayShader.glsl");

	// Generate screen renderer
	m_renderer = std::make_shared<Renderer2D>();

	return true;
}

void ScreenBufferDisplay::draw(Resource<Texture> textureHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Clean buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_screenShader->use();

	textureHandler.get()->bind();

	m_renderer->SetDrawType(Renderer::DrawType::Triangles);
	
	auto mesh = m_quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh().get();

	graphics->mesh = mesh;
	graphics->shader = m_screenShader;
	m_renderer->render();

	//mesh.mesh->render(*m_screenShader, *m_renderer);

	textureHandler.get()->unbind();
}