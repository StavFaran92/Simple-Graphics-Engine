#include "TexturePainter.h"

void TexturePainter::init()
{
    m_texturePaintShader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/TexturePaintShader.glsl");
}

void TexturePainter::setTexture(const ResourceWrapper<Texture>& tex)
{
    m_texture = tex;
}

void TexturePainter::setBrushRadius(float r)
{
    m_brushRadius = r;
}

void TexturePainter::setBrushStrength(float s)
{
    m_brushStrength = s;
}

void TexturePainter::applyBrush(int pixelX, int pixelY)
{
    if (m_texture.isEmpty())
        return;

    GLuint texID = m_texture->getID();
    int w = m_texture->getWidth();
    int h = m_texture->getHeight();

    m_texturePaintShader->use();
    m_texturePaintShader->setUniformValue("brushCenter", glm::vec2((float)pixelX, (float)pixelY));
    m_texturePaintShader->setUniformValue("brushRadius", m_brushRadius);
    m_texturePaintShader->setUniformValue("brushStrength", m_brushStrength);

    glBindImageTexture(0, texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

    GLuint groupX = (GLuint)((w + 15) / 16);
    GLuint groupY = (GLuint)((h + 15) / 16);

    glDispatchCompute(groupX, groupY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

std::vector<float> TexturePainter::readTextureData()
{
    std::vector<float> pixels;

    if (m_texture.isEmpty())
        return pixels;

    int w = m_texture->getWidth();
    int h = m_texture->getHeight();

    pixels.resize(w * h);

    m_texture->bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data());
    m_texture->unbind();

    return pixels;
}
