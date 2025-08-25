#include "component/FoliageComponent.h"

#include "component/Transformation.h"
#include "systems/FoliageSystem.h"
#include "core/Random.h"
#include <GL/glew.h>

#include "component/Terrain.h"
#include <algorithm>
#include "runtime/Scene.h"

void FoliageComponent::build()
{
	m_patchCount = glm::vec2(ceil(width / patchWidth), ceil(height / patchHeight));
	pixelPerPatch = width / m_patchCount.x;

	glm::vec2 ratio = glm::vec2(m_foliageSpreadMap->getWidth() / width, m_foliageSpreadMap->getHeight() / height);

	m_patches.clear();
	m_patches.reserve(m_patchCount.x * m_patchCount.y);
	for (int i = 0; i < m_patchCount.y; i++) // Rows
	{
		for (int j = 0; j < m_patchCount.x; j++) // Cols
		{
			auto patch = std::make_shared<FoliagePatch>();
			patch->pos = glm::vec3(i * patchWidth - width / 2., 0, j * patchHeight - height / 2.);
			patch->idx = j;
			patch->idy = i;
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
				int xOffset = p->idx * pixelPerPatch + j;
				int yOffset = p->idy * pixelPerPatch + i;

				float xRelativeToImageOffset = xOffset * ratio.x;
				float yRelativeToImageOffset = yOffset * ratio.y;

				int xModOffset = (int)xRelativeToImageOffset % m_foliageSpreadMap->getHeight();
				int yModOffset = (int)yRelativeToImageOffset % m_foliageSpreadMap->getWidth();

				int xIndexOffset = xModOffset * m_foliageSpreadMap->getWidth();
				int yIndexOffset = yModOffset;

				int index = (xIndexOffset + yIndexOffset) % pixels.size();
				float density = (float)pixels[index] / 255.f;

				int instanceCount = density * globalDensity * 255 ; // times max instances per texel
				p->instanceCount += instanceCount;
			}
		}
		//p.density = r / 255.f;
	}

	std::vector<glm::vec4> patchInstanceData;
	size_t totalSize = pixelPerPatch * pixelPerPatch * globalDensity * 255;
	patchInstanceData.reserve(totalSize);

	for (int i = 0; i < pixelPerPatch; i++)
	{
		for (int j = 0; j < pixelPerPatch; j++)
		{
			int instanceCount = globalDensity * 255; // times max instances per texel
			for (int k = 0; k < instanceCount; ++k)
			{
				glm::vec3 pos{};
				pos += glm::vec3((float)i * patchWidth / pixelPerPatch, 0, (float)j * patchHeight / pixelPerPatch);	// Offset by texel chunk
				pos += foliageSystem->getRandomLocation(k) * glm::vec3((float)patchWidth / pixelPerPatch, 0, (float)patchHeight / pixelPerPatch);

				patchInstanceData.push_back(glm::vec4(pos, 1.0));
			}
		}
	}
	auto& gen = Engine::get()->getRandomSystem()->getGenerator();
	std::shuffle(patchInstanceData.begin(), patchInstanceData.end(), gen);

	if (m_patchInstanceDataSSBO)
	{
		glDeleteBuffers(1, &m_patchInstanceDataSSBO);
	}
	glGenBuffers(1, &m_patchInstanceDataSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_patchInstanceDataSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * patchInstanceData.size(), patchInstanceData.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_patchInstanceDataSSBO);
}

glm::vec2 FoliageComponent::getPatchCount() const
{
	return m_patchCount;
}

const std::vector<std::shared_ptr<FoliagePatch>>& FoliageComponent::getPatches() const
{
	return m_patches;
}

void FoliageComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto fc = std::dynamic_pointer_cast<FoliageComponent>(c))
	{
		auto& foliage = entityHandler.addComponent<FoliageComponent>(*fc);
		foliage.terrainRef.setRegistry(&scene.getRegistry());
		foliage.build();
	}
}
