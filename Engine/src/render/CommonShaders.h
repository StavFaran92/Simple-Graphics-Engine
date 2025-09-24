#pragma once

#include <memory>
#include <map>
#include "render/Shader.h"
#include "memory/ResourceWrapper.h"

class CommonShaders
{
public:
    enum ShaderType
    {
        REFLECTION_SHADER,
        REFRACTION_SHADER,
        PHONG_SHADER,
        PICKING_SHADER,
        NORMAL_DISPLAY_SHADER
    };

    CommonShaders();

    void close();

    ResourceWrapper<Shader> getShader(ShaderType sType) const;

    CommonShaders(const CommonShaders& other) = delete;
    CommonShaders& operator=(const CommonShaders& other) = delete;

private:
    std::map<ShaderType, ResourceWrapper<Shader>> m_shaders;
    
};