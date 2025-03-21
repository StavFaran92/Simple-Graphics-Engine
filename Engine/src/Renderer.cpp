#include "Renderer.h"

#include <GL/glew.h>
#include "glm/glm.hpp"

#include "VertexArrayObject.h"
#include "Shader.h"
#include "ICamera.h"
#include "EditorCamera.h"
#include "SkyboxRenderer.h"
#include "Logger.h"
#include "Mesh.h"
#include "Entity.h"
#include "Engine.h"
#include "Context.h"
#include "Scene.h"
#include "TimeManager.h"
#include "Entity.h"
#include "Skybox.h"
#include "Transformation.h"
#include "Material.h"
#include "Shader.h"
#include "Component.h"
#include "Transformation.h"
#include "CommonShaders.h"
#include "MeshCollection.h"
#include "Graphics.h"
#include "RenderView.h"
#include "ScreenQuad.h"
#include "RenderCommand.h"
#include "ShapeFactory.h"


Renderer::Renderer(Scene* scene)
    : m_scene(scene)
{
}

bool Renderer::init()
{
	m_pbrShader = Shader::createShared<Shader>(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBRShader.glsl");

    m_quad = ScreenQuad::GenerateScreenQuad(&Engine::get()->getContext()->getRegistry());
    m_quad.RemoveComponent<RenderableComponent>();
    m_quad.RemoveComponent<ObjectComponent>();

    return true;
}

void Renderer::render()
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    // Setup
    graphics->shader->use();
    setUniforms();

    // Draw
    draw(*graphics->mesh->getVAO());


    // Release
    if (graphics->material)
    {
        graphics->material->release();
    }

    graphics->shader->release();
}

void Renderer::enableWireframeMode(bool enable)
{
    m_wireFrameMode = enable;
}

void Renderer::renderScene(Scene* scene)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    graphics->entityGroup.clear();
    for (auto&& [entity, mesh, transform, renderable] :
        scene->getRegistry().getRegistry().view<MeshComponent, Transformation, RenderableComponent>().each())
    {
        if (renderable.renderTechnique == RenderableComponent::RenderTechnique::Forward)
        {
            Entity entityhandler{ entity, &scene->getRegistry() };
            graphics->entityGroup.push_back(entityhandler);
        }
    }

    glEnable(GL_DEPTH_TEST);
    SetDrawType(Renderer::DrawType::Triangles);

    graphics->renderView->bind();

    // Render Phase
    for (auto& entityHandler : graphics->entityGroup)
	{
        graphics->entity = &entityHandler;
        for (auto& mesh : entityHandler.getComponent<MeshComponent>().mesh.get()->getMeshes())
        {

            auto tempModel = entityHandler.getComponent<Transformation>().getWorldTransformation();
            graphics->model = &tempModel;
            graphics->shader = Engine::get()->getCommonShaders()->getShader(CommonShaders::ShaderType::PHONG_SHADER).get();
            graphics->mesh = mesh.get();

            // TODO rethink this feature
            Shader* attachedShader = graphics->entity->tryGetComponentInParent<Shader>();
            if (attachedShader)
            {
                graphics->shader = attachedShader;
            }

            Material* mat = graphics->entity->tryGetComponentInParent<Material>();

            if (mat)
            {
                graphics->material = mat;
            }

            // draw model
            render();
        }
    };

    //graphics->renderView->unbind();
}

void Renderer::setUniforms()
{

    auto graphics = Engine::get()->getSubSystem<Graphics>();

    //auto context = Engine::get()->getContext();
    //if (context->getActiveScene()->getSkybox() && entity->HasComponent<Material>())
    //{
    //    auto mat = entity->getComponent<Material>();
    //    if (mat.isReflective())
    //    {
    //        shaderToUse = context->GetReflectionShader();
    //        shaderToUse->use();
    //        shaderToUse->setValue("skybox", 0);
    //        auto textures = context->getActiveScene()->getSkybox()->getTextureHandlers();
    //        if (textures.size() <= 0)
    //        {
    //            logError("Skybox does not contain cubemap texture.");
    //            return;
    //        }
    //        textures[0]->bind();
    //    }

    //    if (mat.isRefractive())
    //    {
    //        shaderToUse = context->GetRefractiveShader();
    //        shaderToUse->use();
    //        shaderToUse->setValue("skybox", 0);
    //        shaderToUse->setValue("refractiveRatio", 1 / 1.52f);
    //        auto textures = context->getActiveScene()->getSkybox()->getTextureHandlers();
    //        if (textures.size() <= 0)
    //        {
    //            logError("Skybox does not contain cubemap texture.");
    //            return;
    //        }
    //        textures[0]->bind();
    //    }
    //}

    // Model
    if (graphics->model)
    {
        graphics->shader->setModelMatrix(*graphics->model);
    }

    // View
    if (graphics->view)
    {
        graphics->shader->setViewMatrix(*graphics->view);
    }

    // Projection
    if (graphics->projection)
    {
        graphics->shader->setProjectionMatrix(*graphics->projection);
    }

    if (graphics->material)
    {
        graphics->material->use(*graphics->shader);
    }

    graphics->shader->bindUniformBlockToBindPoint("Time", 0);
    graphics->shader->bindUniformBlockToBindPoint("Lights", 1);


}

void Renderer::renderSceneUsingCustomShader(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	glEnable(GL_DEPTH_TEST);

	// Filter objects to acquire only custom shader objects
	for (auto&& [entity/*, mesh*/, transform, renderable, shader] :
		scene->getRegistry().getRegistry().view</*MeshComponent, */Transformation, RenderableComponent, ShaderComponent>().each())
	{
		Entity entityHandler{ entity, &scene->getRegistry() };

        graphics->entity = &entityHandler;

        // bind shader
        auto& shaderComponent = graphics->entity->getComponent<ShaderComponent>();
        Shader* fragmentShader = shaderComponent.m_fragmentShader ?
            shaderComponent.m_fragmentShader : m_pbrShader.get();
        fragmentShader->use();
        graphics->shader = fragmentShader;

        // Bind mesh
        Resource<MeshCollection> meshCollecton;

        if (shader.projection == ShaderComponent::DefaultProjection)
        {
            meshCollecton = entityHandler.getComponent<MeshComponent>().mesh;
        }
        else if (shader.projection == ShaderComponent::Texture2D)
        {
            meshCollecton = m_quad.getComponent<MeshComponent>().mesh;
        }

		// fill bone animation data
		auto animator = entityHandler.tryGetComponent<Animator>();
		if (!animator || animator->m_currentAnimation.isEmpty())
		{
			graphics->shader->setUniformValue("isAnimated", false);
		}
		else
		{
			std::vector<glm::mat4> finalBoneMatrices;
			animator->getFinalBoneMatrices(meshCollecton.get(), finalBoneMatrices);
			for (int i = 0; i < finalBoneMatrices.size(); ++i)
			{
				graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
			}

			graphics->shader->setUniformValue("isAnimated", true);
		}

		

		for (auto mesh : meshCollecton.get()->getMeshes())
		{
			graphics->mesh = mesh.get();
			auto& transform = graphics->entity->getComponent<Transformation>();
			graphics->model = &transform.getWorldTransformation();

			// TODO get this to work
			AABB& aabb = mesh.get()->getAABB();
			aabb.adjustToTransform(transform);

			if (!aabb.isOnFrustum(*graphics->frustum))
			{
				//continue; todo fix
			}

            fragmentShader->bindUniformBlockToBindPoint("Time", 0);
            fragmentShader->bindUniformBlockToBindPoint("Lights", 1);

            fragmentShader->setUniformValue("cameraPos", graphics->cameraPos);
            fragmentShader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);

			auto matIndex = mesh->getMaterialIndex();
			auto materialComponent = graphics->entity->tryGetComponent<MaterialComponent>();
            if (!materialComponent)
            {
                graphics->material = Engine::get()->getDefaultMaterial().get();
            }
            else
            {
                graphics->material = materialComponent->at(matIndex).get();
                if (!graphics->material)
                {
                    graphics->material = Engine::get()->getDefaultMaterial().get();
                }
            }

			{
				int currentSlot = 8;
				for (const auto& [texName, texture] : shaderComponent.customTextures)
				{
					texture.get()->setSlot(currentSlot);
					texture.get()->bind();
					fragmentShader->setUniformValue(texName, currentSlot);
					currentSlot++;
				}
			}

			// if texture projection is enabled bind to custom FBO
            if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
            {
                graphics->renderView = shaderComponent.renderViewProjection;
            }

            graphics->renderView->bind();

			{
				// render to quad
				auto& mesh = m_quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh();
				RenderCommand::draw(mesh->getVAO());
			}

			graphics->renderView->unbind();
		}
	}

}
