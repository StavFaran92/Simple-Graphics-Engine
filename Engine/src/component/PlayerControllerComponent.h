#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI PlayerController : public Component
{
	PlayerController() = default;

	std::string getName() override { return "PlayerController"; }

	template <class Archive>
	void serialize(Archive& archive) {
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
	int controllerIndex = 0;

};

REGISTER_COMPONENT(PlayerController)