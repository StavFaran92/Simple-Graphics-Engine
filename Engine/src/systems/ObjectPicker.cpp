#include "systems/ObjectPicker.h"

#include "GL/glew.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "core/Window.h"
#include "render/FrameBufferObject.h"
#include "render/RenderBufferObject.h"
#include "memory/ResourceWrapper.h"
#include "ui/Input.h"
#include "render/IRenderer.h"
#include "camera/ICamera.h"
#include "render/PickingShader.h"
#include "runtime/Entity.h"
#include "texture/Texture.h"
#include "render/Shader.h"
#include "component/Component.h"
#include "render/RenderCommand.h"
#include "geometry/ShapeFactory.h"
#include "component/Transformation.h"
#include "geometry/MeshCollection.h"
#include "render/FrameBufferObject.h"
#include "component/CameraComponent.h"
#include "component/MeshComponent.h"
#include "component/SkyboxComponent.h"

#include "core/Logger.h"

ObjectPicker::ObjectPicker()
{
	Engine::get()->registerSubSystem<ObjectPicker>(this);
}

bool ObjectPicker::init()
{
	m_pickingShader = Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/PickingShader.glsl");

	m_frameBuffer = std::make_shared<FrameBufferObject>();

	// Bind FBO
	m_frameBuffer->bind();

	auto activeScene = Engine::get()->getContext()->getActiveScene();;

	// Get window width and height
	int width = Engine::get()->getWindow()->getWidth();
	int height = Engine::get()->getWindow()->getHeight();

	// Create a empty texture and attach to FBO
	m_targetTexture = Texture::createEmptyTexture(width, height, GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT);
	m_frameBuffer->attachTexture(m_targetTexture.get()->getID());

	// Create RBO and attach to FBO
	RenderBufferObject rbo{ width, height };
	m_frameBuffer->attachRenderBuffer(rbo.GetID(), FrameBufferObject::AttachmentType::Depth);

	// validate FBO
	if (!m_frameBuffer->isComplete())
	{
		logError("Framebuffer is not complete!");
		return false;
	}

	// Cleanup
	m_frameBuffer->unbind();

	return true;
}

int ObjectPicker::pickObject(int x, int y, Entity camera)
{
	if (!camera.HasComponent<CameraComponent>())
	{
		logWarning("Entity does not have camera component!");
		return -1;
	}

	auto& primaryCamera = camera.getComponent<CameraComponent>();
	auto& primaryCameraTransform = camera.getComponent<Transformation>();

	auto view = glm::lookAt(primaryCameraTransform.getWorldPosition(), primaryCameraTransform.getWorldPosition() + primaryCamera.front, primaryCamera.up);

	m_frameBuffer->bind();

	m_pickingShader->use();

	// set viewport
	glViewport(0, 0, Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight());

	RenderCommand::clear();

	auto activeScene = Engine::get()->getContext()->getActiveScene();

	m_pickingShader->setUniformValue("projection", activeScene->getProjection());
	m_pickingShader->setUniformValue("view", view);

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

	m_frameBuffer->unbind();

	if (pixel.ObjectID != 0)
	{
		logInfo("Selected Object: " + std::to_string(pixel.ObjectID));
		m_selectedObject = pixel.ObjectID;
	}
	else
	{
		m_selectedObject = -1;
	}

	return m_selectedObject;


}

int ObjectPicker::getSelectedObject() const
{
	return m_selectedObject;
}

void ObjectPicker::setSelectedObject(int selectedID)
{
	m_selectedObject = selectedID;
}