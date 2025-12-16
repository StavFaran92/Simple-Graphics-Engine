#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"
#include <vector>
#include <glm/glm.hpp>

struct FoliagePatch
{
	glm::vec3 pos{};
	float density = 0;
	int idx = 0;
	int idy = 0;
	int LOD = 0;
	std::vector<glm::vec4> instancesData;
	int instanceCount = 0;
};

struct EngineAPI FoliageField
{
	FoliageField() = default;

	void build();

	glm::vec2 getPatchCount() const;

	//std::shared_ptr<FoliagePatch> getPatch(int idx, int idy);

	void setPixel(int idx, int idy, unsigned char value);

	void update();

	const std::vector<std::shared_ptr<FoliagePatch>>& getPatches() const;

	template <class Archive>
	void serialize(Archive& archive) {
	    SERIALIZED_MEMBER(m_foliageSpreadMap);
	    SERIALIZED_MEMBER(globalDensity);
	    SERIALIZED_MEMBER(colorA);
	    SERIALIZED_MEMBER(colorB);
	    SERIALIZED_MEMBER(patchWidth);
	    SERIALIZED_MEMBER(patchHeight);
	    SERIALIZED_MEMBER(width);
	    SERIALIZED_MEMBER(height);
	}

	std::vector<unsigned char> m_foliageSpreadMap;
	float globalDensity = 1.f;
	glm::vec3 colorA = glm::vec3(0.1, 0.3, 0.1);
	glm::vec3 colorB = glm::vec3(0.4, 0.8, 0.3);
	int patchWidth = 1;
	int patchHeight = 1;
	int pixelPerPatch = 1;
	float width = 10;
	float height = 10;
	float heightScale = 1;
	ResourceWrapper<Texture> foliageHeightMap;

private:

	std::vector<std::shared_ptr<FoliagePatch>> m_patches;
	glm::vec2 m_patchCount;
	unsigned int m_patchInstanceDataSSBO;
	
	glm::vec2 ratio{ 1.0f, 1.0f };
	
};