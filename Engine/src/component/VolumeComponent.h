#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

class EngineAPI VolumeComponent : public Component
{
public:
	VolumeComponent() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(material);
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&);

	AssetHandle<Material> material;
	AssetHandle<MeshGroupAsset> mesh;
};

REGISTER_COMPONENT(VolumeComponent)