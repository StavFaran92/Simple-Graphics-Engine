#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "render/Material.h"
#include "geometry/Model.h"

class EngineAPI VolumeComponent : public Component
{
public:
	VolumeComponent() = default;

	std::string getName() override { return "VolumeComponent"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(material);
	}

	MaterialAssetRef material;
	ModelAssetRef mesh;
};

REGISTER_COMPONENT(VolumeComponent)