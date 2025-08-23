#pragma once

#include "core/Core.h"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "serialize/CerealHelpers.h"
#include <vector>

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

struct EngineAPI FoliageComponent : public Component
{
	FoliageComponent() = default;

	void build();

	glm::vec2 getPatchCount() const;

	const std::vector<FoliagePatch>& getPatches() const;

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

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
	    SERIALIZED_MEMBER(terrainRef);
	}

	Resource<Texture> m_foliageSpreadMap;
	float globalDensity = 1.f;
	glm::vec3 colorA = glm::vec3(0.1, 0.3, 0.1);
	glm::vec3 colorB = glm::vec3(0.4, 0.8, 0.3);
	int patchWidth = 10;
	int patchHeight = 10;
	int pixelPerPatch = 1;
	float width = 10;
	float height = 10;

	Entity terrainRef = Entity::EmptyEntity;

private:

	std::vector<FoliagePatch> m_patches;
	glm::vec2 m_patchCount;
	unsigned int m_patchInstanceDataSSBO;
	
};


REGISTER_COMPONENT(FoliageComponent)