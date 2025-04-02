#include "ObjectPicker.h"

#include "GL/glew.h"

#include "Engine.h"
#include "Context.h"
#include "Scene.h"
#include "Window.h"
#include "FrameBufferObject.h"
#include "RenderBufferObject.h"
#include "Resource.h"
#include "Input.h"
#include "IRenderer.h"
#include "ICamera.h"
#include "PickingShader.h"
#include "Entity.h"
#include "Texture.h"
#include "Shader.h"
#include "Component.h"
#include "RenderCommand.h"
#include "ShapeFactory.h"
#include "Transformation.h"
#include "MeshCollection.h"

#include "Logger.h"

ObjectPicker::ObjectPicker()
{
	Engine::get()->registerSubSystem<ObjectPicker>(this);
}

bool ObjectPicker::init()
{
	m_pickingShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PickingShader.glsl");

	// Bind FBO
	m_frameBuffer.bind();

	auto activeScene = Engine::get()->getContext()->getActiveScene();;

	// Get window width and height
	int width = Engine::get()->getWindow()->getWidth();
	int height = Engine::get()->getWindow()->getHeight();

	// Create a empty texture and attach to FBO
	m_targetTexture = Texture::createEmptyTexture(width, height, GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT);
	m_frameBuffer.attachTexture(m_targetTexture.get()->getID());

	// Create RBO and attach to FBO
	RenderBufferObject rbo{ width, height };
	m_frameBuffer.attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	// validate FBO
	if (!m_frameBuffer.isComplete())
	{
		logError("Framebuffer is not complete!");
		return false;
	}

	// Cleanup
	m_frameBuffer.unbind();

	return true;
}

int ObjectPicker::pickObject(int x, int y)
{
	m_frameBuffer.bind();

	m_pickingShader->use();

	// set viewport
	glViewport(0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight());

	RenderCommand::clear();

	auto activeScene = Engine::get()->getContext()->getActiveScene();

	m_pickingShader->setUniformValue("projection", activeScene->getProjection());
	m_pickingShader->setUniformValue("view", activeScene->getActiveCameraView());

	for (auto& [entity, meshComponent, transform] : activeScene->getRegistry().getRegistry().view<MeshComponent, Transformation>().each())
	{
		Entity entityhandler{ entity, &activeScene->getRegistry() };
		if (entityhandler.HasComponent<SkyboxComponent>())
			continue;
		m_pickingShader->setUniformValue("objectIndex", (unsigned int)entityhandler.handlerID());
		m_pickingShader->setUniformValue("model", transform.getWorldTransformation());

		for (auto& mesh : meshComponent.mesh.get()->getMeshes())
		{

			auto vao = mesh->getVAO();

			// render to quad
			RenderCommand::draw(vao);
		}
	}

	PixelInfo pixel;
	glReadPixels(x, Engine::get()->getWindow()->getHeight() - y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &pixel);

	m_frameBuffer.unbind();

	if (pixel.ObjectID != 0)
	{
		logInfo("Selected Object: " + std::to_string(pixel.ObjectID));

		return pixel.ObjectID;
	}

	return -1;


}