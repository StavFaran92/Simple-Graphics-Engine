#include "render/PostProcessProjector.h"

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

PostProcessProjector::PostProcessProjector(Scene* scene)
{
	m_scene = scene;

	scene->addRenderCallback(Scene::RenderPhase::PRE_RENDER_BEGIN, [=]() {

		// Post process Enable writing
		if (isEnabled())
		{
			enableWriting();
		}
	});

	scene->addRenderCallback(Scene::RenderPhase::POST_RENDER_END, [=]() {

		// Post process Enable writing
		if (isEnabled())
		{
			disableWriting();
			draw();
		}
	});
}

bool PostProcessProjector::init(int windowWidth, int windowHeight)
{
	// Create FBO
	m_frameBuffer = std::make_shared<FrameBufferObject>();
	m_frameBuffer->bind();

	// Get window width and height
	auto width = windowWidth;
	auto height = windowHeight;

	// Create a empty texture and attach to FBO
	m_textureHandler = Resource<Texture>(Texture::createEmptyTexture(width, height));
	m_frameBuffer->attachTexture(m_textureHandler.get()->getID());

	// Create RBO and attach to FBO
	m_renderBuffer = std::make_shared<RenderBufferObject>(1024, 768);
	m_frameBuffer->attachRenderBuffer(m_renderBuffer->GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	// validate FBO
	if (!m_frameBuffer->isComplete())
	{
		logError("Framebuffer is not complete!");
		return false;
	}
	
	// Cleanup
	m_frameBuffer->unbind();

	// Generate screen quad
	m_quad = ScreenQuad::GenerateScreenQuad(&m_scene->getRegistry());
	
	// Generate screen shader
	m_screenShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_default.glsl");

	// Generate screen renderer
	m_renderer = std::make_shared<Renderer2D>();

	return true;
}

void PostProcessProjector::enableWriting()
{
	m_frameBuffer->bind();

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessProjector::disableWriting()
{
	m_frameBuffer->unbind();
	glDisable(GL_DEPTH_TEST);
}

void PostProcessProjector::draw()
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Clean buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_screenShader->use();

	m_textureHandler.get()->bind();

	m_renderer->SetDrawType(Renderer::DrawType::Triangles);
	
	auto& mesh = m_quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh();

	graphics->mesh = mesh.get();
	graphics->shader = m_screenShader;
	m_renderer->render();

	m_textureHandler.get()->unbind();
}

void PostProcessProjector::setPostProcessShader(Resource<Shader> shader)
{
	m_screenShader = shader;
}

bool PostProcessProjector::isEnabled() const
{
	return m_isEnabled;
}

void PostProcessProjector::setEnabled(bool enable)
{
	m_isEnabled = enable;
}
