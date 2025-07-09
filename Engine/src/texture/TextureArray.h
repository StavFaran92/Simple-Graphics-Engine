#pragma once

#include <vector>
#include <string>

class TextureArray
{
public:
    //TextureArray();
    TextureArray(int width, int height, int channels);
    TextureArray(const std::vector<std::string>& texturePaths);

    void addTexture(const std::string& texturePath);
    void setTexture(int slot, const void* data);
    void removeTexture(int slot);

private:
    inline static const int maxTextureCount = 16;

    int m_currentTextureCount = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
    unsigned int textureID = 0;
};