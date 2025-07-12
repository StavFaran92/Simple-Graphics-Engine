#pragma once

#include "Core.h"

#include "Component.h"

struct EngineAPI FoliageComponent : public Component
{
	FoliageComponent() = default;

	Resource<Texture> m_foliageSpreadMap;

	std::vector<std::shared_ptr<Transformation>> transformations;

	std::vector<glm::mat4> matrices;

	void build();
	
};
