#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI PlayerController : public Component
{
	PlayerController() = default;

	std::string getName() override { return "PlayerController"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER_OPTIONAL(height, 1.f);
		SERIALIZED_MEMBER_OPTIONAL(radius, .5f);
		SERIALIZED_MEMBER_OPTIONAL(offset, glm::vec3{});
	}

	void move(glm::vec3 disp)
	{
		m_disp = disp;
	}

	void reset()
	{
		m_disp = glm::vec3(0.f);
	}

	glm::vec3 m_disp{};
	float height = 1.f;
	float radius = .5f;
	glm::vec3 offset{};
	int controllerIndex = 0;

};

REGISTER_COMPONENT(PlayerController)