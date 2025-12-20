#include "render/Renderer.h"

#include <GL/glew.h>
#include "glm/glm.hpp"

#include "render/VertexArrayObject.h"
#include "render/Shader.h"
#include "camera/ICamera.h"
#include "core/Logger.h"
#include "geometry/Mesh.h"
#include "runtime/Entity.h"
#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "systems/TimeManager.h"
#include "systems/Skybox.h"
#include "component/Transformation.h"
#include "render/Material.h"
#include "render/CommonShaders.h"
#include "geometry/MeshCollection.h"
#include "render/Graphics.h"
#include "render/RenderView.h"
#include "render/ScreenQuad.h"
#include "render/RenderCommand.h"
#include "geometry/ShapeFactory.h"
#include "component/MeshRendererComponent.h"

#include "component/RenderableComponent.h"
#include "component/ObjectComponent.h"
#include "component/ShaderComponent.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"

bool Renderer::init()
{
	//m_pbrShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBRShader.glsl");

    m_quad = BuiltInAssets::getByName<MeshCollection>(SGE_MESH_QUAD).resource();

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

    for (auto&& [entity, meshRenderer, transform, obj] :
        scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
    {
        if (meshRenderer.renderTechnique != MeshRendererComponent::RenderTechnique::Forward)
            continue;

        Entity entityHandler{ entity, &scene->getRegistry() };

        prepareEntityForRender(entityHandler);

        for (auto& mesh : meshRenderer.mesh.get()->getMeshes())
        {

            if (!prepareMeshForRender(mesh.get(), entityHandler))
            {
                continue;
            }

            // draw model
                
            graphics->shader = graphics->material->getActiveShader();
            graphics->shader->use();
            setUniforms();

            // Draw
            RenderCommand::draw(mesh->getVAO());
        }
    }
}

void Renderer::renderSceneNonOpaque(Scene* scene)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    graphics->renderView->bind();

    std::map<float, Entity> transparentEntities;

    auto& camera = graphics->renderView->getCamera();
    auto& camTransform = camera.getComponent<Transformation>();
    auto& camForward = camTransform.getForward();

    for (auto&& [entity, mesh, transform, obj] :
        scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
    {
        Entity entityHandler{ entity, &scene->getRegistry() };
        

        auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

        for (auto& mesh : meshRenderer.mesh.get()->getMeshes())
        {            
            float distance = glm::dot(transform.getWorldPosition(), camForward);

            // object is behind the camera
            if (distance < 0)
            {
                //continue; // TODO fix
            }

            transparentEntities[distance] = entityHandler;
        }
    }

    graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_FORWARD_PBR);
    auto iter = transparentEntities.rbegin();
    while (iter != transparentEntities.rend())
    {
        Entity& entityHandler = iter->second;

        std::string name = entityHandler.getComponent<ObjectComponent>().name;
        std::string captionGPU = "About to render: '" + name + "'";
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

        prepareEntityForRender(entityHandler);

        graphics->entity = entityHandler;
        graphics->shader->use();
        for (auto& mesh : entityHandler.getComponent<MeshRendererComponent>().mesh.get()->getMeshes())
        {

            if (!prepareMeshForRender(mesh.get(), entityHandler))
            {
                continue;
            }

            // Only render transparent objects
            if (graphics->material->getMaterialRenderMode() != MaterialRenderMode::Transparent)
            {
                continue;
            }

            // draw model
            glm::mat3 transposeInverseModelMatrix = glm::mat3(glm::transpose(glm::inverse(graphics->model)));
            graphics->shader->setUniformValue("transposeInverseModelMatrix", transposeInverseModelMatrix);
            setUniforms();

            std::string captionSubmeshGPU = "About to render submesh: '" + mesh->getName() + "' using material: '" + graphics->material->getName() + "'";
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionSubmeshGPU.c_str());

            // Draw
            RenderCommand::draw(graphics->mesh->getVAO());

            glPopDebugGroup();
        }

        glPopDebugGroup();

        iter++;
    }
}

void Renderer::setUniforms()
{

    auto graphics = Engine::get()->getSubSystem<Graphics>();

    graphics->shader = graphics->material->getActiveShader();

    graphics->shader->setModelMatrix(graphics->model);
    graphics->shader->setViewMatrix(graphics->view);
    graphics->shader->setProjectionMatrix(graphics->projection);
    graphics->material->use();

    graphics->shader->bindUniformBlockToBindPoint("Time", 0);
    graphics->shader->bindUniformBlockToBindPoint("Lights", 1);
    graphics->shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 6);
    graphics->shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 7);
    graphics->shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 8);

    graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);
}

void Renderer::renderDebugData(Scene* scene)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    glEnable(GL_DEPTH_TEST);
    graphics->renderView->bind();

    for (auto&& [entity, meshRenderer, transform, obj] :
        scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
    {
        Entity entityHandler{ entity, &scene->getRegistry() };
        std::string name = entityHandler.getComponent<ObjectComponent>().name;
        logTrace("About to display debug data for {}", name);

        for (auto& mesh : meshRenderer.mesh.get()->getMeshes())
        {

            if (!prepareMeshForRender(mesh.get(), entityHandler))
            {
                continue;
            }

            graphics->shader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_DEBUG_DATA);
            graphics->shader->use();

            graphics->shader->setModelMatrix(graphics->model);
            graphics->shader->setViewMatrix(graphics->view);
            graphics->shader->setProjectionMatrix(graphics->projection);
            graphics->shader->bindUniformBlockToBindPoint("Time", 0);
            graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);

            // Draw
            RenderCommand::draw(mesh->getVAO());
        }
    }
}

void Renderer::renderSceneUsingCustomShader(Scene* scene)
{

    auto graphics = Engine::get()->getSubSystem<Graphics>();

    glEnable(GL_DEPTH_TEST);
    graphics->renderView->bind();

    for (auto&& [entity, meshRenderer, transform, obj] :
        scene->getRegistry().getRegistry().view<MeshRendererComponent, Transformation, ObjectComponent>().each())
    {
        Entity entityHandler{ entity, &scene->getRegistry() };
        std::string name = entityHandler.getComponent<ObjectComponent>().name;
        logTrace("About to render '{}' using Custom Shader pass", name);

        for (auto& mesh : meshRenderer.mesh.get()->getMeshes())
        {

            if (!prepareMeshForRender(mesh.get(), entityHandler))
            {
                continue;
            }

            if (graphics->material->getMaterialRenderMode() != MaterialRenderMode::Custom)
                continue;

            //if (graphics->material->m_shader.get()->getShaderOverride() != ShaderOverride::PBR)
            //{
            //    logWarning("Only PBR shader override is supported ATM.");
            //    continue;
            //}

            // draw model

            graphics->shader = graphics->material->getActiveShader();
            graphics->shader->use();
            glm::mat3 transposeInverseModelMatrix = glm::mat3(glm::transpose(glm::inverse(graphics->model)));
            graphics->shader->setUniformValue("transposeInverseModelMatrix", transposeInverseModelMatrix);
            setUniforms();

            // Draw
            RenderCommand::draw(mesh->getVAO());
        }
    }

	//auto graphics = Engine::get()->getSubSystem<Graphics>();

	//// Filter objects to acquire only custom shader objects
	//for (auto&& [entity, transform, renderable, shaderComponent] :
	//	scene->getRegistry().getRegistry().view<Transformation, RenderableComponent, ShaderComponent>().each())
	//{
	//	Entity entityHandler{ entity, &scene->getRegistry() };

 //       if (entityHandler.HasComponent<VolumeComponent>()) continue; // todo fix

 //       graphics->entity = entityHandler;

 //       // bind shader
 //       auto& shaderComponent = graphics->entity.getComponent<ShaderComponent>();
 //       if (!shaderComponent.isValid)
 //       {
 //           continue;

 //       }
 //       ResourceWrapper<Shader> shader = shaderComponent.m_customShader.resource();
 //       shader->use();
 //       graphics->shader = shader;

 //       if (shaderComponent.shaderOverride == ShaderOverride::PBR)
 //       {
 //           // Bind mesh
 //           ResourceWrapper<MeshCollection> meshCollecton;

 //           if (shaderComponent.projection == ShaderComponent::DefaultProjection)
 //           {
 //               meshCollecton = entityHandler.getComponent<MeshRendererComponent>().mesh.resource();
 //           }
 //           else if (shaderComponent.projection == ShaderComponent::Texture2D)
 //           {
 //               meshCollecton = m_quad;
 //           }

 //           // fill bone animation data
 //           auto animator = entityHandler.tryGetComponent<Animator>();
 //           if (!animator || animator->m_currentAnimation.isEmpty())
 //           {
 //               graphics->shader->setUniformValue("isAnimated", false);
 //           }
 //           else
 //           {
 //               std::vector<glm::mat4> finalBoneMatrices;
 //               animator->getFinalBoneMatrices(meshCollecton.get(), finalBoneMatrices);
 //               for (int i = 0; i < finalBoneMatrices.size(); ++i)
 //               {
 //                   graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
 //               }

 //               graphics->shader->setUniformValue("isAnimated", true);
 //           }

 //           graphics->shader->bindUniformBlockToBindPoint("Time", 0);
 //           graphics->shader->bindUniformBlockToBindPoint("Lights", 1);

 //           graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);
 //           graphics->shader->setUniformValue("lightSpaceMatrix", graphics->lightSpaceMatrix);

 //           graphics->shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 5);
 //           graphics->shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 6);
 //           graphics->shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 7);



 //           for (auto mesh : meshCollecton.get()->getMeshes())
 //           {
 //               if (!prepareMeshForRender(mesh.get(), entityHandler))
 //               {
 //                   continue;
 //               }

 //               {
 //                   int currentSlot = 8;
 //                   for (const auto& [texName, texture] : shaderComponent.customTextures)
 //                   {
 //                       texture.get()->setSlot(currentSlot);
 //                       texture.get()->bind();
 //                       graphics->shader->setUniformValue(texName, currentSlot);
 //                       currentSlot++;
 //                   }
 //               }

 //               // if texture projection is enabled bind to custom FBO
 //               if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
 //               {
 //                   graphics->renderView = shaderComponent.renderViewProjection;
 //                   graphics->renderView->bind();
 //                   auto& mesh = m_quad.get()->getPrimaryMesh();
 //                   RenderCommand::draw(mesh->getVAO());
 //               }


 //               if (shaderComponent.projection == ShaderComponent::ProjectionType::DefaultProjection)
 //               {
 //                   graphics->renderView->bind();
 //                   glm::mat3 transposeInverseModelMatrix = glm::mat3(glm::transpose(glm::inverse(graphics->model)));
 //                   graphics->shader->setUniformValue("transposeInverseModelMatrix", transposeInverseModelMatrix);
 //                   setUniforms();
 //                   RenderCommand::draw(mesh->getVAO());
 //               }
 //           }
 //       }

 //       else if (shaderComponent.shaderOverride == ShaderOverride::Pixel)
 //       {
 //           graphics->shader->bindUniformBlockToBindPoint("Time", 0);
 //           graphics->shader->setUniformValue("cameraPos", graphics->cameraPos);
 //           glm::vec3 camView = {};// todo get cam view from view matrix
 //           graphics->shader->setUniformValue("cameraLookAt", camView);

 //           {
 //               int currentSlot = 8;
 //               for (const auto& [texName, texture] : shaderComponent.customTextures)
 //               {
 //                   texture.get()->setSlot(currentSlot);
 //                   texture.get()->bind();
 //                   graphics->shader->setUniformValue(texName, currentSlot);
 //                   currentSlot++;
 //               }
 //           }

 //           // Bind mesh
 //           ResourceWrapper<MeshCollection> meshCollecton;

 //           if (shaderComponent.projection == ShaderComponent::DefaultProjection)
 //           {
 //               meshCollecton = entityHandler.getComponent<MeshRendererComponent>().mesh.resource();
 //           }
 //           else if (shaderComponent.projection == ShaderComponent::Texture2D)
 //           {
 //               meshCollecton = m_quad;
 //           }

 //           for (auto mesh : meshCollecton.get()->getMeshes())
 //           {
 //               graphics->mesh = mesh.get();
 //               auto& transform = graphics->entity.getComponent<Transformation>();
 //               graphics->model = transform.getWorldTransformation() * mesh->getRestTransform();

 //               // TODO get this to work
 //               AABB& aabb = mesh.get()->getAABB();
 //               aabb.transform(graphics->model);

 //               if (!aabb.isOnFrustum(*graphics->frustum))
 //               {
 //                   continue; 
 //               }

 //               // if texture projection is enabled bind to custom FBO
 //               if (shaderComponent.projection == ShaderComponent::ProjectionType::Texture2D)
 //               {
 //                   glDisable(GL_DEPTH_TEST);

 //                   graphics->renderView = shaderComponent.renderViewProjection;
 //               }

 //               graphics->renderView->bind();

 //               {
 //                   // render to quad
 //                   auto& mesh = m_quad.get()->getPrimaryMesh();
 //                   RenderCommand::draw(mesh->getVAO());
 //               }

 //               graphics->renderView->unbind();
 //           }
 //       }

 //       
	//}

}
