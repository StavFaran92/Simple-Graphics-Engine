#pragma once
#include "Core.h"
#include "Resource.h"

class Shader;

class EngineAPI PostProcess
{
public:
	static Resource<Shader> inversion();
	static Resource<Shader> grayscale();
};
