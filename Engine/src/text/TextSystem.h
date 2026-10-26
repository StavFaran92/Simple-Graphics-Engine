#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include "memory/AssetAliases.h"
#include "systems/SubSystem.h"

struct TextComponent;

struct Glyph
{
	glm::vec4 plane{}; // left, bottom, right, top in em, relative to the pen on the baseline
	glm::vec4 uv{};    // left, bottom, right, top
	float advance = 0;
	bool visible = false;
};

struct Font
{
	TextureResourceRef atlas;
	float lineHeight = 0;
	float ascender = 0;
	float distanceRange = 0;
	float emSize = 0;
	std::array<Glyph, 128> glyphs{};
};

class EngineAPI TextSystem : public SubSystem
{
public:
	TextSystem() = default;
	TextSystem(const TextSystem&) = delete;
	TextSystem& operator=(const TextSystem&) = delete;

	bool init();

	const std::vector<std::string>& getFontNames() const { return m_fontNames; }

	// Unknown or empty names fall back to the default font
	const Font* getFont(const std::string& fontName);

	void drawText(TextComponent& text, const glm::mat4& projection);

private:
	ShaderResourceRef m_shader;
	std::vector<std::string> m_fontNames;
	std::unordered_set<std::string> m_warnedFontNames;
	std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;
};
