#include "render/ScreenBufferDisplay.h"

#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "render/Renderer2D.h"
#include "render/ScreenQuad.h"
#include "core/Engine.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "memory/ResourceWrapper.h"
#include "runtime/Scene.h"
#include "runtime/Entity.h"
#include "component/Component.h"
#include "texture/Texture.h"
#include "geometry/MeshCollection.h"
#include "render/Renderer.h"
#include "render/Graphics.h"
#include "component/MeshComponent.h"

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
	ShaderAssetDescriptor aDesc;
	aDesc.isEngineOwned = true;
	m_screenShader = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/ScreenBufferDisplayShader.glsl", aDesc);

	// Generate screen renderer
	m_renderer = std::make_shared<Renderer2D>();

	return true;
}

void ScreenBufferDisplay::draw(ResourceWrapper<Texture> textureHandler)
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