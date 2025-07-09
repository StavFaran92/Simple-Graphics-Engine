#include "PostProcess.h"

#include "Engine.h"

#include "Shader.h"

Resource<Shader> PostProcess::inversion()
{ 
	return Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_inversion.glsl");
}
Resource<Shader> PostProcess::grayscale()
{
	return Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/PostProcess/PostProcessShader_grayscale.glsl");
}
