#include "systems/ShadowSystem.h"

#include "render/FrameBufferObject.h"
#include "memory/ResourceRef.h"
#include "gl/glew.h"
#include "core/Logger.h"
#include "runtime/Scene.h"
#include "component/Component.h"
#include "core/Engine.h"
#include "core/Window.h"
#include "lights/DirectionalLight.h"
#include "component/Transformation.h"
#include "render/Shader.h"
#include "runtime/Context.h"
#include "texture/Texture.h"
#include "animation/Animator.h"
#include "geometry/Model.h"
#include "render/Graphics.h"
#include "render/RenderCommand.h"
#include "geometry/ShapeFactory.h"
#include "utils/DebugHelper.h"
#include "component/MeshRendererComponent.h"
#include "component/RenderableComponent.h"


ShadowSystem::ShadowSystem()
{}

bool ShadowSystem::init()
{
	//m_scene->addRenderCallback(Scene::RenderPhase::PRE_RENDER_BEGIN, [=](const IRenderer::DrawQueueRenderParams* params) {

	//	renderToDepthMap(params);
	//});

	auto graphics = Engine::get()->getSubSystem<Graphics>();
	if (!createDepthMap(graphics->shadowSettings.resolution))
		return false;

	m_simpleDepthShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SimpleDepthShader.glsl");

	//m_bufferDisplay = std::make_shared<ScreenBufferDisplay>(m_scene);
	//m_bufferDisplay->init(Engine::get()->getWindow()->getWidth(), Engine::get()->getWindow()->getHeight());

	return true;
}

bool ShadowSystem::createDepthMap(int resolution)
{
	m_fbo.bind();

	// Generate 2D texture
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
	textureData.width = resolution;
	textureData.height = resolution;
	textureData.channels = 1;
	textureData.internalFormat = TextureInternalFormat::DEPTH_COMPONENT;
	textureData.format = TextureFormat::DEPTH_COMPONENT;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	m_depthMapTexture = Texture::createTexture(textureData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach texture to FBO
	m_fbo.attachTexture(m_depthMapTexture.get()->getID(), GL_DEPTH_ATTACHMENT);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (!m_fbo.isComplete())
	{
		logError("FBO is not complete!");
		m_fbo.unbind();
		return false;
	}

	m_fbo.unbind();

	m_resolution = resolution;

	DebugHelper::getInstance().registerTextureForDebug("Depth map", m_depthMapTexture);

	return true;
}

void ShadowSystem::renderToDepthMap(Scene* scene, const std::vector<DrawItem>& drawItems)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();
	const ShadowSettings& settings = graphics->shadowSettings;

	if (settings.resolution > 0 && settings.resolution != m_resolution)
		createDepthMap(settings.resolution);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	// Set shadow map viewport
	glViewport(0, 0, m_resolution, m_resolution);

	// Bind FBO
	m_fbo.bind();

	// Clear buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	auto view =scene->getRegistry().getRegistry().view<DirectionalLight>();
	if (view.size() < 1)
	{
		// No directional light, for now simply return
		m_fbo.unbind();
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		return;
	}

	// Configure Shadow pass matrices
	auto entt = view.front();
	Entity e{ entt, &scene->getRegistry() };
	auto& dirLight = e.getComponent<DirectionalLight>();

	//todo verify exists

	// Generate Orthogonal projection
	glm::mat4 lightProjection = glm::ortho(
		settings.left, settings.right,
		settings.bottom, settings.top,
		settings.nearPlane, settings.farPlane);

	auto& trans = e.getComponent<Transformation>();
	auto& direction = trans.getLocalRotationVec3();

	auto right = glm::normalize(glm::cross(direction, { 0,0,1 }));
	auto up = glm::normalize(glm::cross(direction, right));

	// Generate lookAt light matrix 
	glm::mat4 dirLightView = glm::lookAt(
		settings.lightOrigin,
		settings.lightOrigin + direction,
		up);

	m_lightSpaceMatrix = lightProjection * dirLightView;

	m_simpleDepthShader->use();
	m_simpleDepthShader->setUniformValue("lightSpaceMatrix", m_lightSpaceMatrix);

	const auto& bones = scene->getDrawItemBones();

	// Render regular items
	m_simpleDepthShader->setUniformValue("isGpuInstanced", false);

	for (const auto& item : drawItems)
	{
		if (item.flags & DRAW_ITEM_INSTANCED)
			continue;

		if (item.flags & DRAW_ITEM_ANIMATED)
		{
			for (uint32_t i = 0; i < item.boneCount; ++i)
			{
				m_simpleDepthShader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", bones[item.boneOffset + i]);
			}

			m_simpleDepthShader->setUniformValue("isAnimated", true);
		}
		else
		{
			m_simpleDepthShader->setUniformValue("isAnimated", false);
		}

		m_simpleDepthShader->setUniformValue("model", item.transform);

		// TODO use a more sophisticated solution here
		//if (!item.worldBounds.isOnFrustum(*graphics->frustum))
		//{
		//	continue;
		//}

		RenderCommand::draw(item.mesh->getVAO());
	}

	// Render instanced items, bone transforms were computed in Scene::generateDrawItems
	m_simpleDepthShader->setUniformValue("isGpuInstanced", true);

	graphics->instancedModelBuffer.setSlot(0);
	graphics->instancedModelBuffer.bind();

	graphics->instancedAnimationBuffer.setSlot(1);
	graphics->instancedAnimationBuffer.bind();

	graphics->instancedInstanceDataBuffer.setSlot(2);
	graphics->instancedInstanceDataBuffer.bind();

	for (const auto& item : drawItems)
	{
		if (!(item.flags & DRAW_ITEM_INSTANCED))
			continue;

		m_simpleDepthShader->setUniformValue("restTransform", item.mesh->getRestTransform());
		m_simpleDepthShader->setUniformValue("instanceOffset", static_cast<int>(item.instanceOffset));

		RenderCommand::drawInstanced(item.mesh->getVAO(), item.instanceCount);
	}

	// Unbind FBO
	m_fbo.unbind();

	// Set viewport back to normal
	//auto width = Engine::get()->getWindow()->getWidth();
	//auto height = Engine::get()->getWindow()->getHeight();
	//glViewport(0, 0, width, height);

	//auto phongShader = m_context->getStandardShader();

	//// set lightSpaceMatrix in shader
	//phongShader->setValue("lightSpaceMatrix", lightSpaceMatrix);

	////TODO Fix
	//// set Depth map texture in shader
	//phongShader->setValue("shadowMap", 2);
	//m_depthMapTexture->setSlot(2);
	//m_depthMapTexture->bind();

	//m_bufferDisplay->draw(m_depthMapTexture);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
}

TextureResourceRef ShadowSystem::getShadowMap() const
{
	return m_depthMapTexture;
}

glm::mat4 ShadowSystem::getLightSpaceMat() const
{
	return m_lightSpaceMatrix;
}
