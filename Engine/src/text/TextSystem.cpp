#include "text/TextSystem.h"

#include <chrono>

#include "text/FontAtlasGenerator.h"
#include "component/TextComponent.h"
#include "fileSystem/FileSystem.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "core/Logger.h"

namespace
{
	const std::string DEFAULT_FONT = SGE_ROOT_DIR "Resources/Engine/Fonts/Play-Regular.ttf";
	constexpr float ATLAS_EM_SIZE = 48;
	constexpr float ATLAS_DISTANCE_RANGE = 4;
}

bool TextSystem::init()
{
	m_shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/TextShader.glsl");

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return !m_shader.isEmpty();
}

const Font* TextSystem::getFont(const std::string& fontPath)
{
	auto it = m_fonts.find(fontPath);
	if (it != m_fonts.end())
		return it->second.get();

	// A failed font stays cached as null so the error is logged once, not every frame
	std::unique_ptr<Font>& font = m_fonts[fontPath];

	std::string ttfPath = fontPath.empty() ? DEFAULT_FONT : FileSystem::toAbsolute(fontPath).string();

	auto start = std::chrono::steady_clock::now();

	FontAtlasData data;
	if (!generateFontAtlas(ttfPath, ATLAS_EM_SIZE, ATLAS_DISTANCE_RANGE, data))
		return nullptr;

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

	font = std::make_unique<Font>();
	font->lineHeight = data.lineHeight;
	font->ascender = data.ascender;
	font->distanceRange = data.distanceRange;
	font->emSize = data.emSize;

	glm::vec4 atlasSize(data.width, data.height, data.width, data.height);
	Glyph fallback;
	for (const FontAtlasGlyph& g : data.glyphs)
	{
		if (g.codepoint >= font->glyphs.size())
			continue;

		Glyph& glyph = font->glyphs[g.codepoint];
		glyph.plane = g.plane;
		glyph.uv = g.atlas / atlasSize;
		glyph.advance = g.advance;
		glyph.visible = !g.whitespace;

		if (g.codepoint == '?')
			fallback = glyph;
	}

	for (size_t i = 0; i < font->glyphs.size(); i++)
	{
		if (font->glyphs[i].advance == 0)
			font->glyphs[i] = fallback;
	}

	ImageBuffer pixels;
	pixels = std::move(data.pixels);
	font->atlas = Texture::createTexture(data.width, data.height, 4,
		TextureInternalFormat::RGBA, TextureFormat::RGBA, TextureType::UNSIGNED_BYTE,
		TextureFilter::Linear, TextureWrap::Clamp, pixels);

	logInfo("Font atlas for {}: {}x{}, {} glyphs, {} ms", ttfPath, data.width, data.height, data.glyphs.size(), ms);

	return font.get();
}

void TextSystem::drawText(const TextComponent& text, const glm::mat4& projection)
{
	const Font* font = getFont(text.fontPath);
	if (!font)
		return;

	const float size = text.fontSize;
	float penX = text.position.x;
	float baseline = text.position.y + font->ascender * size;

	m_vertices.clear();

	for (char c : text.text)
	{
		if (c == '\n')
		{
			penX = text.position.x;
			baseline += font->lineHeight * size;
			continue;
		}

		unsigned char index = (unsigned char)c;
		const Glyph& glyph = font->glyphs[index < font->glyphs.size() ? index : '?'];

		if (glyph.visible)
		{
			// Metrics are y up, the UI projection is y down
			float left = penX + glyph.plane.x * size;
			float right = penX + glyph.plane.z * size;
			float top = baseline - glyph.plane.w * size;
			float bottom = baseline - glyph.plane.y * size;

			glm::vec4 topLeft(left, top, glyph.uv.x, glyph.uv.w);
			glm::vec4 topRight(right, top, glyph.uv.z, glyph.uv.w);
			glm::vec4 bottomLeft(left, bottom, glyph.uv.x, glyph.uv.y);
			glm::vec4 bottomRight(right, bottom, glyph.uv.z, glyph.uv.y);

			m_vertices.insert(m_vertices.end(), { topLeft, bottomLeft, bottomRight, topLeft, bottomRight, topRight });
		}

		penX += glyph.advance * size;
	}

	if (m_vertices.empty())
		return;

	m_shader->use();
	m_shader->setProjectionMatrix(projection);
	m_shader->setUniformValue("color", text.color);
	m_shader->setUniformValue("screenPxRange", glm::max(font->distanceRange * size / font->emSize, 1.0f));
	m_shader->setTextureInShader(font->atlas, "atlas", 0);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec4), m_vertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
