#include "component/CameraComponent.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

glm::mat4 CameraComponent::getProjection() const
{
	if (type == CamType::PERSPECTIVE)
	{
		float fov = getFOVYInRadians();
		return glm::perspective(fov, aspect, znear, zfar);

	}
	else if (type == CamType::ORTHOGRAPHIC)
	{
		return glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top);

	}
	else
	{
		return glm::mat4(1.f);

	}

}