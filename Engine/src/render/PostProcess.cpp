#include "render/PostProcess.h"

#include "core/Engine.h"

#include "render/Shader.h"

ResourceWrapper<Shader> PostProcess::inversion()
{ 
	return Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_inversion.glsl");
}
ResourceWrapper<Shader> PostProcess::grayscale()
{
	return Shader::loadTransient(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_grayscale.glsl");
}
