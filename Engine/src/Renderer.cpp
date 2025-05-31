#include "Renderer.h"

#include <GL/glew.h>
#include "glm/glm.hpp"

#include "VertexArrayObject.h"
#include "Shader.h"
#include "ICamera.h"
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
	m_pbrShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBRShader.glsl");

    m_quad = ScreenQuad::GenerateScreenQuad(&Engine::get()->getContext()->getRegistry());
    m_quad.RemoveComponent<RenderableComponent>();
    m_quad.RemoveComponent<ObjectComponent>();

    return true;
}

void Renderer::enableWireframeMode(bool enable)
{
    m_wireFrameMode = enable;
}

void Renderer::renderScene(Scene* scene)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    glEnable(GL_DEPTH_TEST);
    graphics->renderView->bind();

    for (auto&& [entity, mesh, transform, renderable] :
        scene->getRegistry().getRegistry().view<MeshComponent, Transformation, RenderableComponent>(entt::exclude<ShaderComponent>).each())
    {
        if (renderable.renderTechnique == RenderableComponent::RenderTechnique::Forward)
        {
            Entity entityHandler{ entity, &scene->getRegistry() };

            graphics->entity = &entityHandler;
            for (auto& mesh : entityHandler.getComponent<MeshComponent>().mesh.get()->getMeshes())
            {

                auto tempModel = entityHandler.getComponent<Transformation>().getWorldTransformation();
                graphics->model = &tempModel;
                graphics->shader = m_pbrShader;
                graphics->mesh = mesh.get();

                Material* mat = graphics->entity->tryGetComponentInParent<Material>();

                if (mat)
                {
                    graphics->material = mat;
                }

                // draw model
                graphics->shader->use();
                setUniforms();

                // Draw
                draw(*graphics->mesh->getVAO());
            }
        }
    }
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
        graphics->material->use(graphics->shader);
    }

    graphics->shader->bindUniformBlockToBindPoint("Time", 0);
    graphics->shader->bindUniformBlockToBindPoint("Lights", 1);
}

void Renderer::render()
{
}

void Renderer::renderSceneUsingCustomShader(Scene* scene)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Filter objects to acquire only custom shader objects
	for (auto&& [entity, transform, renderable, shaderComponent] :
		scene->getRegistry().getRegistry().view<Transformation, RenderableComponent, ShaderComponent>().each())
	{
		Entity entityHandler{ entity, &scene->getRegistry() };

        if (entityHandler.HasComponent<VolumeComponent>()) continue; // todo fix

        graphics->entity = &entityHandler;

        // bind shader
        auto& shaderComponent = graphics->entity->getComponent<ShaderComponent>();
        if (!shaderComponent.isValid)
        {
            continue;

        }
        Resource<Shader> shader = shaderComponent.m_customShader;
        shader->use();
        graphics->shader = shader;

        if (shaderComponent.shaderOverride == ShaderOverride::PBR)
        {
            // Bind mesh
            Resource<MeshCollection> meshCollecton;

            if (shaderComponent.projection == ShaderComponent::DefaultProjection)
            {
                meshCollecton = entityHandler.getComponent<MeshComponent>().mesh;
            }
            else if (shaderComponent.projection == ShaderComponent::Texture2D)
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

                graphics->shader->bindUniformBlockToBindPoint("Time", 0);
                graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

                graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);
                graphics->shader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);

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
                        graphics->shader->setUniformValue(texName, currentSlot);
                        currentSlot++;
                    }
                }

                // if texture projection is enabled bind to custom FBO
                if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
                {
                    graphics->renderView = shaderComponent.renderViewProjection;
                    graphics->renderView->bind();
                    auto& mesh = m_quad.getComponent<MeshComponent>().mesh.get()->getPrimaryMesh();
                    RenderCommand::draw(mesh->getVAO());
                }


                if (shaderComponent.projection == ShaderComponent::ProjectionType::DefaultProjection)
                {
                    graphics->renderView->bind();
                    setUniforms();
                    RenderCommand::draw(mesh->getVAO());
                }
            }
        }

        else if (shaderComponent.shaderOverride == ShaderOverride::Pixel)
        {
            graphics->shader->bindUniformBlockToBindPoint("Time", 0);
            graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);
            glm::vec3 camView = {};// todo get cam view from view matrix
            graphics->shader->setUniformValue("cameraLookAt", camView);

            {
                int currentSlot = 8;
                for (const auto& [texName, texture] : shaderComponent.customTextures)
                {
                    texture.get()->setSlot(currentSlot);
                    texture.get()->bind();
                    graphics->shader->setUniformValue(texName, currentSlot);
                    currentSlot++;
                }
            }

            // Bind mesh
            Resource<MeshCollection> meshCollecton;

            if (shaderComponent.projection == ShaderComponent::DefaultProjection)
            {
                meshCollecton = entityHandler.getComponent<MeshComponent>().mesh;
            }
            else if (shaderComponent.projection == ShaderComponent::Texture2D)
            {
                meshCollecton = m_quad.getComponent<MeshComponent>().mesh;
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

                // if texture projection is enabled bind to custom FBO
                if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
                {
                    glDisable(GL_DEPTH_TEST);

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

}
