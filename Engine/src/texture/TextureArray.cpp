#include "TextureArray.h"

#include <GL/glew.h>
#include "stb_image.h"
#include <cassert>

TextureArray::TextureArray(int width, int height, int channels)
    : m_width(width), m_height(height), m_channels(channels)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 255, GL_RGBA, m_width, m_height, maxTextureCount);

    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void TextureArray::addTexture(const std::string& texturePath)
{
    assert(m_currentTextureCount < maxTextureCount);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    assert(m_width == width && m_height == height && m_channels == channels);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, m_currentTextureCount++, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}

void TextureArray::setTexture(int slot, const void* data)
{
    assert(slot >= 0 && slot < maxTextureCount && slot < m_currentTextureCount);

    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, slot, m_width, m_height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void TextureArray::removeTexture(int slot)
{
    assert(slot >= 0 && slot < maxTextureCount);

    for (int i = 0; i < slot; i++)
    {
        //TBD
    }
}