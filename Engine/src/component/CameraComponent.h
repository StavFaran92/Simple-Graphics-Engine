#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI CameraComponent : public Component
{
	CameraComponent() = default;

	std::string getName() override { return "CameraComponent"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(center);
		SERIALIZED_MEMBER(up);
		SERIALIZED_MEMBER(fovyRadians);
		SERIALIZED_MEMBER(aspect);
		SERIALIZED_MEMBER(znear);
		SERIALIZED_MEMBER(zfar);
		SERIALIZED_MEMBER(type);
		SERIALIZED_MEMBER(ortho_left);
		SERIALIZED_MEMBER(ortho_right);
		SERIALIZED_MEMBER(ortho_bottom);
		SERIALIZED_MEMBER(ortho_top);
	}

	static CameraComponent createPerspectiveCamera(float fovy, float aspect, float znear, float zfar)
	{
		CameraComponent cam;
		cam.fovyRadians = glm::radians(fovy);
		cam.aspect = aspect;
		cam.znear = znear;
		cam.zfar = zfar;
		cam.type = CamType::PERSPECTIVE;
		return cam;
	}

	float getFOVYInDegrees() const
	{
		return glm::degrees(fovyRadians);
	}
	float getFOVYInRadians() const
	{
		return fovyRadians;
	}
	void setFOVY(float FOVY)
	{
		fovyRadians = glm::radians(FOVY);
	}

	glm::mat4 getProjection() const;

	
	float fovyRadians = 0; // Should be accessed through the getters and setters avaiable
	float aspect = 0;
	float znear = 0;
	float zfar = 0;

	// Only avaiable to orthographic projection
	float ortho_left = 0;
	float ortho_right = 10;
	float ortho_bottom = 0;
	float ortho_top = 10;

	enum CamType
	{
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

	CamType type;

	glm::vec3 front{ 0,0,-1 };
	glm::vec3 right;
	glm::vec3 center{ 0,0,0 };
	glm::vec3 up{ 0,1,0 };
};

REGISTER_COMPONENT(CameraComponent)