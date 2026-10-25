#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI TextComponent : public Component
{
	TextComponent() = default;

	std::string getName() override { return "TextComponent"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(text);
		SERIALIZED_MEMBER(fontPath);
		SERIALIZED_MEMBER(position);
		SERIALIZED_MEMBER(fontSize);
		SERIALIZED_MEMBER(color);
	}

	std::string text = "Text";
	std::string fontPath; // .ttf, project-relative; empty = engine default font
	glm::vec2 position{};
	float fontSize = 32;
	glm::vec4 color{ 1, 1, 1, 1 };
};

REGISTER_COMPONENT(TextComponent)
