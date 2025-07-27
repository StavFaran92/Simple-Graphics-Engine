#include "component/FoliageComponent.h"

#include "component/Transformation.h"
#include "systems/FoliageSystem.h"
#include "core/Random.h"

void FoliageComponent::build()
{
	m_patchCount = glm::vec2(ceil(width / patchWidth), ceil(height / patchHeight));

	for (int i = 0; i < m_patchCount.x; i++)
	{
		for (int j = 0; j < m_patchCount.y; j++)
		{
			FoliagePatch patch;
			patch.pos = glm::vec3(i * patchWidth, 0, j * patchHeight);
			m_patches.push_back(patch);

		}

	}
}

glm::vec2 FoliageComponent::getPatchCount() const
{
	return m_patchCount;
}

const std::vector<FoliagePatch>& FoliageComponent::getPatches() const
{
	return m_patches;
}
