#include "CommonShaders.h"

#include "Engine.h"

CommonShaders::CommonShaders()
{
	m_shaders[ShaderType::REFLECTION_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/ReflectionShader.glsl");
	m_shaders[ShaderType::REFRACTION_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/ReflectionShader.glsl");
	m_shaders[ShaderType::PICKING_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/PickingShader.glsl");
}

void CommonShaders::close()
{
	m_shaders.clear();
}

Resource<Shader> CommonShaders::getShader(ShaderType sType) const
{
	return Resource<Shader>();
}
