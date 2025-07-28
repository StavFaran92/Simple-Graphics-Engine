#include "component/FoliageComponent.h"

#include "component/Transformation.h"
#include "systems/FoliageSystem.h"
#include "core/Random.h"
#include <GL/glew.h>

void FoliageComponent::build()
{
	

	m_patchCount = glm::vec2(ceil(width / patchWidth), ceil(height / patchHeight));

	m_patches.clear();
	m_patches.reserve(m_patchCount.x * m_patchCount.y);
	for (int i = 0; i < m_patchCount.x; i++)
	{
		for (int j = 0; j < m_patchCount.y; j++)
		{
			FoliagePatch patch;
			patch.pos = glm::vec3(i * patchWidth, 0, j * patchHeight);
			patch.idx = j;
			patch.idy = i;
			m_patches.push_back(patch);

		}

	}

	m_foliageSpreadMap->bind();
	std::vector<GLubyte> pixels(m_foliageSpreadMap->getWidth() * m_foliageSpreadMap->getHeight() * m_foliageSpreadMap->getBitDepth());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	for (auto& p : m_patches)
	{
		int index = (p.idy * width + p.idx) * 3;
		GLubyte r = pixels[index];
		p.density = r / 255.f;
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
