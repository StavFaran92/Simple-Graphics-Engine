#pragma once

#include "core/Core.h"

#include "component/Component.h"

struct FoliagePatch
{
	glm::vec3 pos{};
	float density = 0;
	int idx = 0;
	int idy = 0;
	int LOD = 0;
};

struct EngineAPI FoliageComponent : public Component
{
	FoliageComponent() = default;

	void build();

	glm::vec2 getPatchCount() const;

	const std::vector<FoliagePatch>& getPatches() const;

	Resource<Texture> m_foliageSpreadMap;
	float density = 0.2f;
	glm::vec3 colorA = glm::vec3(0.22f, 0.55f, 0.13f);
	glm::vec3 colorB = glm::vec3(0.60f, 0.52f, 0.14f);
	int patchWidth = 1;
	int patchHeight = 1;
	int pixelPerPatch = 1;
	float width = 10;
	float height = 10;

private:

	std::vector<FoliagePatch> m_patches;
	glm::vec2 m_patchCount;
};
