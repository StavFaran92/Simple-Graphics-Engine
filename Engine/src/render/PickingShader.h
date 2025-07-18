#pragma once
#include "render/Shader.h"
class PickingShader : public Shader
{
public:
	PickingShader();



	void setObjectIndex(unsigned int index);
	void setDrawIndex(unsigned int index);
};

