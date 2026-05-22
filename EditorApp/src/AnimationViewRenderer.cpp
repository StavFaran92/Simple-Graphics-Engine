#include "AnimationViewRenderer.h"

#include "animation/Animator.h"
#include "animation/Animation.h"
#include "geometry/Model.h"
#include "geometry/Mesh.h"
#include "render/Shader.h"
#include "render/RenderCommand.h"
#include "texture/Texture.h"
#include "core/Logger.h"
#include "core/ApplicationConstants.h"
#include "component/MeshRendererComponent.h"
#include "memory/BuiltInAssets.h"
#include "render/Graphics.h"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

AnimationViewRenderer::AnimationViewRenderer()
{
    m_shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBRShader.glsl");
}

void AnimationViewRenderer::resize(uint32_t width, uint32_t height)
{
    if (width == m_width && height == m_height) return;
    m_width  = width;
    m_height = height;

    // Color texture
    TextureData tData;
    tData.width          = width;
    tData.height         = height;
    tData.channels       = 3;
    tData.internalFormat = TextureInternalFormat::RGB;
    tData.format = TextureFormat::RGB;
    tData.type = TextureType::UNSIGNED_BYTE;
    tData.filter         = TextureFilter::Linear;
    tData.wrap           = TextureWrap::Clamp;
    m_colorTexture = Texture::createTexture(tData);

    // Depth RBO
    m_rbo = RenderBufferObject((int)width, (int)height);

    // FBO
    m_fbo.bind();
    m_fbo.attachTexture(m_colorTexture->getID());
    m_fbo.attachRenderBuffer(m_rbo.GetID(), FrameBufferObject::AttachmentType::Depth_Stencil);

    if (!m_fbo.isComplete())
    {
        logError("AnimationViewRenderer: FBO is not complete!");
        m_fbo.unbind();
        return;
    }

    m_fbo.unbind();        

    m_view       = glm::lookAt(glm::vec3(50.f, 50.f, 0.f),
                               glm::vec3(0.f, 1.f,  0.f),
                               glm::vec3(0.f, 1.f,  0.f));
    m_projection = glm::perspective(glm::radians(45.f),
                                    (float)width / (float)height,
                                    0.1f, 100.f);
}

void AnimationViewRenderer::render(AnimationAssetRef animation, float currentTime, MeshRendererComponent* meshRendererComponent)
{
    if (m_colorTexture.isEmpty() || !m_shader) return;
    if (!meshRendererComponent) return;

    auto graphics = Engine::get()->getSubSystem<Graphics>();

    auto model = meshRendererComponent->mesh;

    std::string captionGPU = "Animation Preview";
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, captionGPU.c_str());

    m_fbo.bind();
    glViewport(0, 0, (int)m_width, (int)m_height);
    glClearColor(0.15f, 0.15f, 0.15f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::vec3 cameraPos;

    // Build camera from combined AABB of all meshes
    {
        glm::vec3 aabbMin( 1e9f);
        glm::vec3 aabbMax(-1e9f);
        for (const auto& mesh : model.resource()->getMeshes())
        {
            AABB bb = mesh->getAABB();
            aabbMin = glm::min(aabbMin, bb.getMin());
            aabbMax = glm::max(aabbMax, bb.getMax());
        }
        glm::vec3 center  = (aabbMin + aabbMax) * 0.5f;
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        float     radius  = glm::length(extents);
        float     fovY    = glm::radians(45.f);
        float     dist    = radius / tanf(fovY * 0.5f) * 1.2f; // 20% margin

        // Eye slightly elevated, looking at AABB center
        glm::vec3 dir  = glm::normalize(glm::vec3(0.f, 0.3f, 1.f));
        glm::vec3 eye  = center + dir * dist;
        m_view         = glm::lookAt(eye, center, glm::vec3(0.f, 1.f, 0.f));
        m_projection   = glm::perspective(fovY, (float)m_width / (float)m_height,
                                          radius * 0.01f, dist * 4.f);
        cameraPos = eye;
    }

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

    if (!animation.isEmpty() && !animation.resource().isEmpty())
    {
        std::unordered_map<std::string, glm::mat4> intermediateBones;
        animation.resource()->calculateFinalBoneMatrices(currentTime, intermediateBones);

        auto boneMatrices = model.resource()->getBoneOffsets();
        for (auto& [boneName, animMat] : intermediateBones)
        {
            int id = model.resource()->getBoneID(boneName);
            if (id != -1)
                boneMatrices[id] = animMat * boneMatrices[id];
        }

        for (int i = 0; i < (int)boneMatrices.size(); ++i)
            m_shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", boneMatrices[i]);

        m_shader->setUniformValue("isAnimated", true);
    }
    else
    {
        m_shader->setUniformValue("isAnimated", false);
    }

    for (const auto& mesh : model.resource()->getMeshes())
    {
        auto matIndex = mesh->getMaterialIndex();
        auto materialResource = meshRendererComponent->at(matIndex);

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

    glPopDebugGroup();

    m_fbo.unbind();
}

uint32_t AnimationViewRenderer::getColorTextureID() const
{
    if (m_colorTexture.isEmpty()) return 0;
    return m_colorTexture->getID();
}
