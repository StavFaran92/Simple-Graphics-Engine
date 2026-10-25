#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
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

class TextSystem : public SubSystem
{
public:
	bool init();

	const Font* getFont(const std::string& fontPath);

	void drawText(const TextComponent& text, const glm::mat4& projection);

private:
	ShaderResourceRef m_shader;
	unsigned int m_vao = 0;
	unsigned int m_vbo = 0;
	std::vector<glm::vec4> m_vertices;
	std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;
};
