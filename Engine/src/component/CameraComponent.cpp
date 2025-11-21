#include "component/CameraComponent.h"

glm::mat4 CameraComponent::getProjection() const
{
	if (type == CamType::PERSPECTIVE) return glm::perspective(getFOVYInRadians(), aspect, znear, zfar);
	if (type == CamType::ORTHOGRAPHIC) return glm::ortho(-100, 100, -100, 100);
}