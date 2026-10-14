#include "PreviewRenderer.h"

#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "render/Shader.h"
#include "render/RenderCommand.h"
#include "texture/Texture.h"
#include "core/Logger.h"
#include "core/ApplicationConstants.h"
#include "memory/BuiltInAssets.h"
#include "render/Graphics.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

PreviewRenderer::PreviewRenderer()
{
    m_shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBRShader.glsl");
}

bool PreviewRenderer::render(const PreviewRenderDesc& desc, const TextureResourceRef& target)
{
    if (target.isEmpty() || !m_shader) return false;
    if (desc.model.isEmpty()) return false;

    const int width  = target->getWidth();
    const int height = target->getHeight();
    if (width <= 0 || height <= 0) return false;

    auto graphics = Engine::get()->getSubSystem<Graphics>();

    const ModelResourceRef& model = desc.model;

    // Depth RBO follows the target size
    if (width != m_depthWidth || height != m_depthHeight)
    {
        m_rbo = RenderBufferObject(width, height);
        m_depthWidth  = width;
        m_depthHeight = height;
    }

    m_fbo.bind();
    m_fbo.attachTexture(target->getID());
    m_fbo.attachRenderBuffer(m_rbo.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

    if (!m_fbo.isComplete())
    {
        logError("PreviewRenderer: FBO is not complete, target texture may not be color-renderable");
        m_fbo.unbind();
        return false;
    }

    std::string captionGPU = "Preview Render";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

    glViewport(0, 0, width, height);
    glClearColor(desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::vec3 cameraPos;

    // Build camera from combined AABB of all meshes
    {
        glm::vec3 aabbMin( 1e9f);
        glm::vec3 aabbMax(-1e9f);
        for (const auto& mesh : model->getMeshes())
        {
            AABB bb = mesh->getAABB();
            aabbMin = glm::min(aabbMin, bb.getMin());
            aabbMax = glm::max(aabbMax, bb.getMax());
        }
        glm::vec3 center  = (aabbMin + aabbMax) * 0.5f;
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        float     radius  = glm::length(extents);
        float     fovY    = glm::radians(45.f);
        float     dist    = radius / tanf(fovY * 0.5f);

        // Eye slightly elevated, looking at AABB center
        glm::vec3 dir  = glm::normalize(glm::vec3(1.f, 0.3f, 1.f));
        glm::vec3 eye  = center + dir * dist;
        m_view         = glm::lookAt(eye, center, glm::vec3(0.f, 1.f, 0.f));
        m_projection   = glm::perspective(fovY, (float)width / (float)height,
                                          radius * 0.01f, dist * 4.f);
        cameraPos = eye;

        if (desc.flipY)
            m_projection = glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f)) * m_projection;
    }

    // Mirroring flips triangle winding; swap the front face so culling keeps working
    GLint prevFrontFace = GL_CCW;
    glGetIntegerv(GL_FRONT_FACE, &prevFrontFace);
    if (desc.flipY)
        glFrontFace(prevFrontFace == GL_CCW ? GL_CW : GL_CCW);

    m_shader->use();
    m_shader->setViewMatrix(m_view);
    m_shader->setProjectionMatrix(m_projection);
    m_shader->setModelMatrix(glm::mat4(1.f));

    m_shader->bindUniformBlockToBindPoint("Time", 0);
    m_shader->bindUniformBlockToBindPoint("Lights", 1);
    m_shader->setTextureInShader(graphics->irradianceMap, "gIrradianceMap", 6);
    m_shader->setTextureInShader(graphics->prefilterEnvMap, "gPrefilterEnvMap", 7);
    m_shader->setTextureInShader(graphics->brdfLUT, "gBRDFIntegrationLUT", 8);
    m_shader->setUniformValue("cameraPos", cameraPos);

    if (!desc.boneMatrices.empty())
    {
        for (int i = 0; i < (int)desc.boneMatrices.size(); ++i)
            m_shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", desc.boneMatrices[i]);

        m_shader->setUniformValue("isAnimated", true);
    }
    else
    {
        m_shader->setUniformValue("isAnimated", false);
    }

    for (const auto& mesh : model->getMeshes())
    {
        MaterialResourceRef materialResource;
        auto iter = desc.materials.find(mesh->getMaterialIndex());
        if (iter != desc.materials.end())
            materialResource = iter->second;

        if(materialResource.isEmpty())
            materialResource = BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT).resource();

        //materialResource->use();
        int slot = 9;

        // Set samplers
        for (const auto& [name, sampler] : materialResource->getAllSamplers())
        {
            materialResource->useSamplerInShader(name, sampler, m_shader, slot);
            slot++;
        }

        // Set uniforms
        for (const auto& [name, value] : materialResource->getAllUniforms())
        {
            m_shader->setUniformValue(name, value);
        }

        RenderCommand::draw(mesh->getVAO());
    }

    glFrontFace(prevFrontFace);

    glPopDebugGroup();

    m_fbo.unbind();
    return true;
}
