#include "component/FoliageComponent.h"

#include "component/Transformation.h"
#include "systems/FoliageSystem.h"
#include "core/Random.h"
#include <GL/glew.h>

#include "component/Terrain.h"

void FoliageComponent::build()
{
	m_patchCount = glm::vec2(ceil(width / patchWidth), ceil(height / patchHeight));

	m_patches.clear();
	m_patches.reserve(m_patchCount.x * m_patchCount.y);
	for (int i = 0; i < m_patchCount.y; i++) // Rows
	{
		for (int j = 0; j < m_patchCount.x; j++) // Cols
		{
			FoliagePatch patch;
			patch.pos = glm::vec3(i * patchWidth - width / 2. + .5f, 0, j * patchHeight - height / 2.);
			patch.idx = j;
			patch.idy = i;
			m_patches.push_back(patch);

		}

	}

	Terrain* terrain = nullptr;
	if (terrainRef != Entity::EmptyEntity)
	{
		terrain = terrainRef.tryGetComponent<Terrain>();
	}

	m_foliageSpreadMap->bind();
	std::vector<GLubyte> pixels(m_foliageSpreadMap->getWidth() * m_foliageSpreadMap->getHeight());
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	auto foliageSystem = Engine::get()->getSubSystem<FoliageSystem>();

	for (auto& p : m_patches)
	{
		for (int i = 0; i < pixelPerPatch; i++)
		{
			for (int j = 0; j < pixelPerPatch; j++)
			{
				// Sample density
				int index = (((p.idy * pixelPerPatch + i) % m_foliageSpreadMap->getHeight()) * m_foliageSpreadMap->getWidth() + 
					(p.idx * pixelPerPatch + j) % m_foliageSpreadMap->getWidth()) ;
				float density = (float)pixels[index % pixels.size()] / 255.f;

				int instanceCount = density * 255; // times max instances per texel
				p.instanceCount += instanceCount;
				for (int k = 0; k < instanceCount; ++k) 
				{
					glm::vec3 pos;
					pos = glm::vec3(p.pos);									// Offset by patch position
					pos += glm::vec3((float)i * patchWidth / pixelPerPatch, 0, (float)j * patchHeight / pixelPerPatch);	// Offset by texel chunk
					pos += foliageSystem->getRandomLocation(k) * glm::vec3((float)patchWidth / pixelPerPatch, 0, (float)patchHeight / pixelPerPatch);

					if (terrain)
					{
						float height = terrain->getHeightAtPoint(pos.x, pos.z);
						pos.y += height;
					}

					p.instancesData.push_back(glm::vec4(pos, 1.0));
				}
			}
		}
		//p.density = r / 255.f;
	}

	if (m_patchInstanceDataSSBO)
	{
		glDeleteBuffers(1, &m_patchInstanceDataSSBO);
	}
	glGenBuffers(1, &m_patchInstanceDataSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_patchInstanceDataSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * 255 * pixelPerPatch * pixelPerPatch, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_patchInstanceDataSSBO);
}

glm::vec2 FoliageComponent::getPatchCount() const
{
	return m_patchCount;
}

const std::vector<FoliagePatch>& FoliageComponent::getPatches() const
{
	return m_patches;
}
