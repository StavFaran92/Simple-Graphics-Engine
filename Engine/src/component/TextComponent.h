#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

class TextMesh;

struct EngineAPI TextComponent : public Component
{
	TextComponent() = default;

	std::string getName() override { return "TextComponent"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER(m_text);
		SERIALIZED_MEMBER(m_fontName);
		SERIALIZED_MEMBER(position);
		SERIALIZED_MEMBER(m_fontSize);
		SERIALIZED_MEMBER(color);

		if constexpr (Archive::is_loading::value)
			m_dirty = true;
	}

	const std::string& getText() const { return m_text; }
	const std::string& getFontName() const { return m_fontName; }
	float getFontSize() const { return m_fontSize; }

	void setText(const std::string& text) { m_text = text; m_dirty = true; }
	void setFontName(const std::string& fontName) { m_fontName = fontName; m_dirty = true; }
	void setFontSize(float fontSize) { m_fontSize = fontSize; m_dirty = true; }

	bool isDirty() const { return m_dirty; }
	const std::shared_ptr<TextMesh>& getMesh() const { return m_mesh; }
	void setMesh(const std::shared_ptr<TextMesh>& mesh) { m_mesh = mesh; m_dirty = false; }

	glm::vec2 position{};
	glm::vec4 color{ 1, 1, 1, 1 };

private:
	std::string m_text = "Text";
	std::string m_fontName; // built-in font name (file stem in Engine/Resources/Engine/Fonts); empty = default
	float m_fontSize = 32;

	bool m_dirty = true;
	std::shared_ptr<TextMesh> m_mesh;
};

REGISTER_COMPONENT(TextComponent)
