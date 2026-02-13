#include "render/CommonShaders.h"

#include "core/Engine.h"

CommonShaders::CommonShaders()
{
	m_shaders[ShaderType::REFLECTION_SHADER] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/ReflectionShader.glsl");
	m_shaders[ShaderType::REFRACTION_SHADER] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/RefractionShader.glsl");
	m_shaders[ShaderType::PICKING_SHADER] = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/PickingShader.glsl");
}

void CommonShaders::close()
{
	m_shaders.clear();
}

ResourceWrapper<Shader> CommonShaders::getShader(ShaderType sType) const
{
	return ResourceWrapper<Shader>();
}
