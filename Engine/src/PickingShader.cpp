#include "PickingShader.h"

PickingShader::PickingShader() : Shader("Resources\\Shaders\\PickingShader.vert", "Resources\\Shaders\\PickingShader.frag")
{
}

void PickingShader::setModelMatrix(glm::mat4 model)
{
	setMat4("model", model);
}

void PickingShader::setViewMatrix(glm::mat4 view)
{
	setMat4("view", view);
}

void PickingShader::setProjectionMatrix(glm::mat4 projection)
{
	setMat4("projection", projection);
}

void PickingShader::setObjectIndex(unsigned int index)
{
	setUInt("objectIndex", index);
}

void PickingShader::setDrawIndex(unsigned int index)
{
	setUInt("drawIndex", index);
}
