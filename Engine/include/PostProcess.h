#pragma once
#include "core/Core.h"
#include "memory/Resource.h"

class Shader;

class EngineAPI PostProcess
{
public:
	static Resource<Shader> inversion();
	static Resource<Shader> grayscale();
};
