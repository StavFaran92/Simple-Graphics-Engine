#include "systems/VolumetricSystem.h"

#include "core/Engine.h"
#include "render/Graphics.h"
#include "render/RenderView.h"
#include "component/VolumeComponent.h"
#include "component/CameraComponent.h"
#include "render/Material.h"
#include "render/RenderCommand.h"
#include "memory/BuiltInAssets.h"
#include "runtime/Scene.h"
#include "core/Window.h"

VolumetricSystem::VolumetricSystem()
{
	Engine::get()->registerSubSystem<VolumetricSystem>(this);
}

bool VolumetricSystem::init()
{
	auto width = Engine::get()->getWindow()->getWidth();
	auto height = Engine::get()->getWindow()->getHeight();

	m_renderTargetFBO = std::make_shared<FrameBufferObject>();
	m_renderTargetRBO = std::make_shared<RenderBufferObject>(width, height);

	m_renderTargetFBO->bind();

	TextureData textureData;
	textureData.target = TextureTarget::TEXTURE_2D;
	textureData.width = width;
	textureData.height = height;
	textureData.channels = 4;
	textureData.internalFormat = TextureInternalFormat::RGBA32F;
	textureData.format = TextureFormat::RGBA;
	textureData.type = TextureType::FLOAT;
	textureData.filter = TextureFilter::Linear;
	textureData.wrap = TextureWrap::Clamp;
	textureData.data = nullptr;
	m_renderTargetTexture = Texture::createTexture(textureData);
	m_renderTargetFBO->attachTexture(m_renderTargetTexture.get()->getID(), GL_COLOR_ATTACHMENT0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Create RBO and attach to FBO
	m_renderTargetFBO->attachRenderBuffer(m_renderTargetRBO->GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

	if (!m_renderTargetFBO->isComplete())
	{
		logError("FBO is not complete!");
		return false;
	}

	m_renderTargetFBO->unbind();

	m_renderVolumeIntoSceneShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/RenderVolumeIntoScreenShader.glsl");

	return true;
}

VolumetricSystem* VolumetricSystem::get()
{
	return Engine::get()->getSubSystem<VolumetricSystem>();
}

void VolumetricSystem::drawVolumetric(const VolumeComponent& volume, const glm::mat4& model)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	m_renderTargetFBO->bind();

	TextureResourceRef renderTargetTexture = graphics->renderView->getRenderTargetTexture();

	//TextureResourceRef renderTargetTexture = graphics->renderView->getRenderTargetTexture();
	//renderView->swapToAdditionalTarget();
	//renderView->bind();
	RenderCommand::clear();
	glDisable(GL_DEPTH_TEST);
	// TODO assert post process shader

	if (volume.material.isEmpty())
		return;

	auto& mat = volume.material.resource();

	//mat->getNonPersistentBlock().setTexture("MainTexture", renderTargetTexture);

	mat->setUniformValue("model", model);
	mat->setUniformValue("view", graphics->view);
	mat->setUniformValue("projection", graphics->projection);

	auto viewport = graphics->renderView->getViewport();
	mat->setUniformValue("screenSize", glm::vec2(viewport.w, viewport.h));

	mat->setUniformValue("cameraPos", graphics->cameraPos);

	const Entity& camera = graphics->renderView->getCamera();
	auto& primaryCamera = camera.getComponent<CameraComponent>();
	mat->setUniformValue("cameraLookAt", primaryCamera.front);
	mat->setUniformValue("cameraFov", primaryCamera.fovyRadians);

	mat->setTexture("uMainTexture", renderTargetTexture);

	mat->use();

	// bind mesh
	ModelResourceRef mesh;
	if (!volume.mesh.isEmpty())
	{
		mesh = volume.mesh.resource(); // will not work for hierarchical meshes
	}
	else
	{
		mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX).resource();

	}

	RenderCommand::draw(mesh->getPrimaryMesh()->getVAO());

	// Swap back to main frame buffer
	graphics->renderView->bind();

	// Now blend scene with drawn volumetrics
	m_renderVolumeIntoSceneShader->use();

	
	m_renderVolumeIntoSceneShader->setTextureInShader(renderTargetTexture, "uMainTexture", 0);
	m_renderVolumeIntoSceneShader->setTextureInShader(m_renderTargetTexture, "uVolumeColor", 1);

	auto quadVAO = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_QUAD).resource()->getPrimaryMesh()->getVAO();
	RenderCommand::draw(quadVAO);

	//auto vao = m_quadUI.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh()->getVAO(); //todo change, we start off with a quad

	// in frag shader i need access to mesh extentes & main texture -> set uniforms

	// draw


	//auto srcID = renderView->getRenderTargetFrameBufferID();

	//renderView->swapBackToMainTargetWithCopy();
	//renderView->bind();

	//auto targetID = renderView->getRenderTargetFrameBufferID();

	//RenderCommand::copyFrameBufferData(srcID,
	//	targetID,
	//	RenderCommand::BufferBit::DEPTH_BUFFER_BIT | RenderCommand::BufferBit::COLOR_BUFFER_BIT);



	glEnable(GL_DEPTH_TEST);
}
