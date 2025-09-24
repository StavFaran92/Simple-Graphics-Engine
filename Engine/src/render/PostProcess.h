#pragma once
#include "core/Core.h"
#include "memory/ResourceWrapper.h"

class Shader;

class EngineAPI PostProcess
{
public:
	static ResourceWrapper<Shader> inversion();
	static ResourceWrapper<Shader> grayscale();
};
