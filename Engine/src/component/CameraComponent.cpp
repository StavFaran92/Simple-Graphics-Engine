#include "component/CameraComponent.h"

glm::mat4 CameraComponent::getProjection() const
{
	if (type == CamType::PERSPECTIVE) return glm::perspective(getFOVYInRadians(), aspect, znear, zfar);
	if (type == CamType::ORTHOGRAPHIC) return glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top);
	return glm::mat4(1.f);
}