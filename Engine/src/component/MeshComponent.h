#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI MeshComponent : public Component
{
	MeshComponent(const Resource<MeshCollection>& mesh) : mesh(mesh) {};
	MeshComponent() = default;

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
	{
		attachSimple<MeshComponent>(c, entityHandler);
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(mesh);
		SERIALIZED_MEMBER(materialSlot);

	}

	float materialSlot = 0; // todo this will be used (probably as a list) to support multi material models
	Resource<MeshCollection> mesh = Resource<MeshCollection>::empty;
	
};

REGISTER_COMPONENT(MeshComponent)