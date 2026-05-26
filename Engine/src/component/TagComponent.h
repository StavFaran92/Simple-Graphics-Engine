#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI TagComponent : public Component
{
	std::string getName() override { return "TagComponent"; }

	std::string tag;

	std::string getTag() const;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(tag);
	}
};

REGISTER_COMPONENT(TagComponent)
