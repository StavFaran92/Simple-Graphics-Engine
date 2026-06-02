#include "IndirectRenderer.h"

#include "memory/ResourceRef.h"
#include "gl/glew.h"
#include "core/Engine.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "component/Component.h"
#include "component/Transformation.h"
#include "render/ScreenQuad.h"
#include "runtime/Scene.h"
#include "render/Material.h"
#include "core/Random.h"
#include "render/RenderCommand.h"
#include "runtime/Context.h"
#include "animation/Animator.h"
#include "geometry/Model.h"
#include "render/Graphics.h"
#include "core/System.h"
#include "geometry/ShapeFactory.h"
#include "render/RenderView.h"
#include "utils/DebugHelper.h"
#include "component/MeshRendererComponent.h"

#include "component/ObjectComponent.h"
#include "component/RenderableComponent.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"

void IndirectRenderer::init()
{
}

void IndirectRenderer::renderScene(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	RenderCommand::clear();

	glEnable(GL_DEPTH_TEST);

	graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM);
	graphics->shader->use();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "G-Buffer pass");

	// Render all objects
	for (auto&& [entity, meshRenderer, transform, obj] :
		scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
	{
		if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Deferred)
			continue;

		Entity entityHandler{ entity, &scene->getRegistry() };
		graphics->entity = entityHandler;

		std::string name = entityHandler.getComponent<ObjectComponent>().name;
		std::string captionGPU = "About to render Entity: '" + name + "'";
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

		prepareEntityForRender(entityHandler);

		for (auto& mesh : meshRenderer.mesh.resource()->getMeshes())
		{
			if (!prepareMeshForRender(mesh.get(), entityHandler))
			{
				continue;
			}

			// Only render Opaque objects
			if (graphics->material->getRenderMode() != MaterialRenderMode::Opaque)
			{
				continue;
			}

			std::string captionSubmeshGPU = "About to render submesh: '" + mesh->getName() + "' using material: '" + graphics->material->getName() + "'";
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionSubmeshGPU.c_str());

			// draw model
			render();

			glPopDebugGroup();
		}

		glPopDebugGroup();

	};

	glPopDebugGroup();

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Light pass");

	if (graphics->renderMode == RenderMode::WIREFRAME)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}

	// unbind gBuffer
	m_gBuffer.unbind();


	//glDisable(GL_DEPTH_TEST);

	if (graphics->useSSAO)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SSAO pass");

		glDisable(GL_DEPTH_TEST);

		m_ssaoFBO.bind();
		m_ssaoPassShader->use();
		glClear(GL_COLOR_BUFFER_BIT);

		// SSAO
		m_ssaoPassShader->setTextureInShader(m_positionTextureVS, "gPositionVS", 0);
		m_ssaoPassShader->setTextureInShader(m_normalTextureVS, "gNormalVS", 1);
		m_ssaoPassShader->setTextureInShader(m_ssaoNoiseTexture, "gSSAONoise", 2);

		// TODO remove
		auto width = Engine::get()->getWindow()->getWidth();
		auto height = Engine::get()->getWindow()->getHeight();

		// We set the viewport to half the screen size to improve the SSAO performance
		RenderCommand::setViewport(0, 0, width / 2.f, height / 2.f);

		m_ssaoPassShader->setUniformValue("screenWidth", (int)(width / 2.f));
		m_ssaoPassShader->setUniformValue("screenHeight", (int)(height / 2.f));

		for (unsigned int i = 0; i < 64; ++i)
		{
			m_ssaoPassShader->setUniformValue("ssaoKernel[" + std::to_string(i) + "]", m_ssaoKernel[i]);
		}

		m_ssaoPassShader->setUniformValue("view", graphics->view);
		m_ssaoPassShader->setUniformValue("projection", graphics->projection);

		{
			// render to quad
			auto vao = m_quad->getPrimaryMesh()->getVAO();
			RenderCommand::draw(vao);
		}

		//glClear(GL_COLOR_BUFFER_BIT);

		m_ssaoBlurFBO.bind();
		m_ssaoBlurPassShader->use();

		m_ssaoBlurPassShader->setTextureInShader(m_ssaoColorBuffer, "gSSAOColorBuffer", 0);


		{
			// render to quad
			auto vao = m_quad->getPrimaryMesh()->getVAO();
			RenderCommand::draw(vao);
		}

		glEnable(GL_DEPTH_TEST);

		// We set the viewport back to original size
		RenderCommand::setViewport(0, 0, width, height);

		glPopDebugGroup();
	}

	// bind textures
	// Todo solve slots issue
	ShaderResourceRef lightPassShaderResource = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT);
	lightPassShaderResource->use();
	lightPassShaderResource->setTextureInShader(m_positionTexture, "gPosition", 0);
	lightPassShaderResource->setTextureInShader(m_normalTexture, "gNormal", 1);
	lightPassShaderResource->setTextureInShader(m_albedoTexture, "gAlbedo", 2);
	lightPassShaderResource->setTextureInShader(m_MRATexture, "gMRA", 3);
	lightPassShaderResource->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 4);
	lightPassShaderResource->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 5);
	lightPassShaderResource->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 6);
	lightPassShaderResource->setTextureInShader(graphics->shadowMap, "gShadowMap", 7);
	lightPassShaderResource->setTextureInShader(m_ssaoBlurColorBuffer, "gSSAOColorBuffer", 8);
	lightPassShaderResource->setTextureInShader(m_TangentTexture, "gTangnet", 9);
	lightPassShaderResource->bindUniformBlockToBindPoint("Time", 0);
	lightPassShaderResource->bindUniformBlockToBindPoint("Lights", 1);
	lightPassShaderResource->setUniformValue("cameraPos", graphics->cameraPos);
	lightPassShaderResource->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);
	lightPassShaderResource->setUniformValue("useSSAO", graphics->useSSAO);

	graphics->renderView->bind();

	{
		// render to quad
		auto vao = m_quad->getPrimaryMesh()->getVAO();
		RenderCommand::draw(vao);
	}

	glPopDebugGroup();
}
