#include "render/CommonShaders.h"

#include "core/Engine.h"

CommonShaders::CommonShaders()
{
	ShaderAssetDescriptor aDesc;
	aDesc.isEngineOwned = true;
	m_shaders[ShaderType::REFLECTION_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/ReflectionShader.glsl", aDesc);
	m_shaders[ShaderType::REFRACTION_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/ReflectionShader.glsl", aDesc);
	m_shaders[ShaderType::PICKING_SHADER] = Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/PickingShader.glsl", aDesc);
}

void CommonShaders::close()
{
	m_shaders.clear();
}

ResourceWrapper<Shader> CommonShaders::getShader(ShaderType sType) const
{
	return ResourceWrapper<Shader>();
}
