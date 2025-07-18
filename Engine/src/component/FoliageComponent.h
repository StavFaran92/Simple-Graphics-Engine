#pragma once

#include "core/Core.h"

#include "component/Component.h"

struct EngineAPI FoliageComponent : public Component
{
	FoliageComponent() = default;

	void build();


	Resource<Texture> m_foliageSpreadMap;
	float density = 0.2f;
	glm::vec3 colorA = glm::vec3(0.22f, 0.55f, 0.13f);
	glm::vec3 colorB = glm::vec3(0.60f, 0.52f, 0.14f);
};
