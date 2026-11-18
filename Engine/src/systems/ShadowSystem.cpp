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
#include "component/CameraComponent.h"


ShadowSystem::ShadowSystem()
{}

bool ShadowSystem::init()
{
	//m_scene->addRenderCallback(Scene::RenderPhase::PRE_RENDER_BEGIN, [=](const IRenderer::DrawQueueRenderParams* params) {

	//	renderToDepthMap(params);
	//});

	if (!createDepthMap())
		return false;

	m_simpleDepthShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/SimpleDepthShader.glsl");

	return true;
}

bool ShadowSystem::createDepthMap()
{
	const ShadowSettings& settings = Engine::get()->getSubSystem<Graphics>()->shadowSettings;
	const int resolution = settings.resolution;
	const int cascadeCount = glm::clamp(settings.cascadeCount, 1, MAX_SHADOW_CASCADES);

	m_fbo.bind();

	// One layer per cascade
	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D_ARRAY;
	textureData.width = resolution;
	textureData.height = resolution;
	textureData.depth = cascadeCount;
	textureData.channels = 1;
	textureData.internalFormat = TextureInternalFormat::DEPTH_COMPONENT;
	textureData.format = TextureFormat::DEPTH_COMPONENT;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	m_depthMapTexture = Texture::createTexture(textureData);

	// Outside the map counts as lit
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	// Layer 0 only validates the FBO; renderToDepthMap attaches each layer in turn
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthMapTexture->getID(), 0, 0);

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
	m_cascades.count = cascadeCount;

	return true;
}

void ShadowSystem::computeSplits(float znear)
{
	const ShadowSettings& settings = Engine::get()->getSubSystem<Graphics>()->shadowSettings;

	const float n = std::max(znear, 0.001f);
	const float f = std::max(settings.shadowDistance, n + 0.01f);
	const int count = m_cascades.count;

	// Practical split scheme: blend between uniform and logarithmic splits
	for (int i = 0; i < count; i++)
	{
		const float p = float(i + 1) / float(count);
		const float logSplit = n * std::pow(f / n, p);
		const float uniformSplit = n + (f - n) * p;
		m_cascades.splits[i] = glm::mix(uniformSplit, logSplit, settings.splitLambda);
	}
}

void ShadowSystem::computeCascade(int i, float sliceNear, float sliceFar, const glm::mat4& view,
	const CameraComponent& camera, const glm::vec3& lightDir, const glm::vec3& lightUp)
{
	const ShadowSettings& settings = Engine::get()->getSubSystem<Graphics>()->shadowSettings;

	// Slice corners in world space
	const glm::mat4 sliceProj = glm::perspective(camera.getFOVYInRadians(), camera.aspect, sliceNear, sliceFar);
	const glm::mat4 invViewProj = glm::inverse(sliceProj * view);

	glm::vec3 corners[8];
	int c = 0;
	for (int x = -1; x <= 1; x += 2)
		for (int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2)
			{
				const glm::vec4 p = invViewProj * glm::vec4(float(x), float(y), float(z), 1.0f);
				corners[c++] = glm::vec3(p) / p.w;
			}

	// Bounding sphere: its radius doesn't change when the camera turns, so the texel size stays constant
	glm::vec3 center(0.0f);
	for (const glm::vec3& p : corners)
		center += p;
	center /= 8.0f;

	float radius = 0.0f;
	for (const glm::vec3& p : corners)
		radius = std::max(radius, glm::length(p - center));
	radius = std::ceil(radius * 16.0f) / 16.0f; // float noise would otherwise change it every frame

	const float depthRange = 2.0f * radius + settings.casterMargin;
	const glm::mat4 lightView = glm::lookAt(center - lightDir * (radius + settings.casterMargin), center, lightUp);
	glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius, 0.0f, depthRange);

	// Snap to whole texels so the texel grid stays fixed in the world while the camera moves
	const float halfRes = m_resolution * 0.5f;
	const glm::vec4 origin = lightProj * lightView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec2 originTexels = glm::vec2(origin) * halfRes;
	const glm::vec2 offset = (glm::round(originTexels) - originTexels) / halfRes;
	lightProj[3].x += offset.x;
	lightProj[3].y += offset.y;

	m_cascades.lightSpaceMatrices[i] = lightProj * lightView;

	// biasScale texels of world size, in the [0, 1] depth the map stores
	const float texelWorld = 2.0f * radius / float(m_resolution);
	m_cascades.bias[i] = settings.biasScale * texelWorld / depthRange;
}

void ShadowSystem::renderToDepthMap(Scene* scene, const std::vector<DrawItem>& drawItems,
	const glm::mat4& view, const CameraComponent& camera)
{
	const ShadowSettings& settings = Engine::get()->getSubSystem<Graphics>()->shadowSettings;

	const int wantedCount = glm::clamp(settings.cascadeCount, 1, MAX_SHADOW_CASCADES);
	if (settings.resolution > 0 && (settings.resolution != m_resolution || wantedCount != m_cascades.count))
		createDepthMap();

	auto lights = scene->getRegistry().getRegistry().view<DirectionalLight>();
	const bool hasLight = lights.size() > 0;

	if (hasLight)
	{
		Entity e{ lights.front(), &scene->getRegistry() };
		auto& trans = e.getComponent<Transformation>();

		computeSplits(camera.znear);

		float sliceNear = camera.znear;
		for (int i = 0; i < m_cascades.count; i++)
		{
			computeCascade(i, sliceNear, m_cascades.splits[i], view, camera, trans.getWorldForward(), trans.getWorldUp());
			sliceNear = m_cascades.splits[i];
		}
	}

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);
	glViewport(0, 0, m_resolution, m_resolution);

	m_fbo.bind();
	m_simpleDepthShader->use();

	for (int i = 0; i < m_cascades.count; i++)
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthMapTexture->getID(), 0, i);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Without a light the cleared layers leave everything lit
		if (!hasLight)
			continue;

		m_simpleDepthShader->setUniformValue("lightSpaceMatrix", m_cascades.lightSpaceMatrices[i]);
		drawCasters(scene, drawItems);
	}

	m_fbo.unbind();
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
}

void ShadowSystem::drawCasters(Scene* scene, const std::vector<DrawItem>& drawItems)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

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
}

TextureResourceRef ShadowSystem::getShadowMap() const
{
	return m_depthMapTexture;
}

const ShadowCascades& ShadowSystem::getCascades() const
{
	return m_cascades;
}

void ShadowSystem::setCascadeUniforms(Shader& shader)
{
	const ShadowCascades& cascades = Engine::get()->getSubSystem<Graphics>()->shadowCascades;

	shader.setUniformValue("cascadeCount", cascades.count);
	for (int i = 0; i < cascades.count; i++)
	{
		const std::string index = "[" + std::to_string(i) + "]";
		shader.setUniformValue("lightSpaceMatrices" + index, cascades.lightSpaceMatrices[i]);
		shader.setUniformValue("cascadeSplits" + index, cascades.splits[i]);
		shader.setUniformValue("cascadeBias" + index, cascades.bias[i]);
	}
}
