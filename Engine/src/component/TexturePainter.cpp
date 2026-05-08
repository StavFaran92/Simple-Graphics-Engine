#include "TexturePainter.h"

#include "memory/BuiltInResources.h"

void TexturePainter::setTexture(const TextureResourceRef& tex)
{
    m_texture = tex;
}

void TexturePainter::setBrushRadius(float r)
{
    m_brushRadius = r;
}

float TexturePainter::getBrushRadius() const
{
    return m_brushRadius;
}

void TexturePainter::setBrushStrength(float s)
{
    m_brushStrength = s;
}

float TexturePainter::getBrushStrength() const
{
    return m_brushStrength;
}

void TexturePainter::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;
}

TexturePainter::DrawMode TexturePainter::getDrawMode() const
{
    return m_drawMode;
}

void TexturePainter::applyBrush(int pixelX, int pixelY)
{

    if (m_texture.isEmpty())
        return;

    GLuint texID = m_texture->getID();
    int w = m_texture->getWidth();
    int h = m_texture->getHeight();

    float brushStrength = (m_drawMode == DrawMode::Add) ? m_brushStrength : -m_brushStrength;

    ShaderResourceRef texturePaintShader = BuiltInResources::get<Shader>(SGE_RESOURCE_SHADER_TEXTURE_BRUSH_DEFORM);
    texturePaintShader->use();
    texturePaintShader->setUniformValue("brushCenter", glm::vec2((float)pixelX, (float)pixelY));
    texturePaintShader->setUniformValue("brushRadius", m_brushRadius);
    texturePaintShader->setUniformValue("brushStrength", brushStrength);

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

TextureResourceRef TexturePainter::getBoundTexture()
{
    return m_texture;
}