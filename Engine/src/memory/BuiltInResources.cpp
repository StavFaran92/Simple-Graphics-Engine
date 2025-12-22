#include "BuiltInResources.h"

#include "render/Shader.h"
#include "core/Engine.h"

BuiltInResources::BuiltInResources()
{
    Engine::get()->registerSubSystem<BuiltInResources>(this);
}

void BuiltInResources::loadAllResources()
{
	m_resources[SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_GeomPassShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBR_LightPassShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_FORWARD_PBR] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PBRShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_TERRAIN] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/TerrainShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_DEBUG_DATA] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/normalDisplayShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_FOLIAGE] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/FoliageShader.glsl");
    m_resources[SGE_RESOURCE_SHADER_TEXTURE_BRUSH_DEFORM] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/TexturePaintShader.glsl");
}
