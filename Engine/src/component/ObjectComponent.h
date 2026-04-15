#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI ObjectComponent : public Component
{
	ObjectComponent() = default;
	ObjectComponent(Entity e, const std::string& name) : name(name), e(e) {};

	void resolve(SceneResourceRef& scene) override;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(e);
	}

	std::string name;
	Entity e = Entity::EmptyEntity;

};

REGISTER_COMPONENT(ObjectComponent)