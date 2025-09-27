#include "render/PostProcess.h"

#include "core/Engine.h"

#include "render/Shader.h"

ResourceWrapper<Shader> PostProcess::inversion()
{ 
	ShaderAssetDescriptor aDesc;
	aDesc.isTransient = true;
	return Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_inversion.glsl", aDesc);
}
ResourceWrapper<Shader> PostProcess::grayscale()
{
	ShaderAssetDescriptor aDesc;
	aDesc.isTransient = true;
	return Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_grayscale.glsl", aDesc);
}
