#include "text/TextSystem.h"

#include <chrono>

#include "text/FontAtlasGenerator.h"
#include "text/TextMesh.h"
#include "component/TextComponent.h"
#include "render/Shader.h"
#include "texture/Texture.h"
#include "core/Logger.h"

namespace
{
	const std::string FONTS_DIR = SGE_ROOT_DIR "Resources/Engine/Fonts/";
	const std::string DEFAULT_FONT_NAME = "Play-Regular";
	constexpr float ATLAS_EM_SIZE = 48;
	constexpr float ATLAS_DISTANCE_RANGE = 4;
}

bool TextSystem::init()
{
	m_shader = Shader::load(SGE_ROOT_DIR "Resources/Engine/Shaders/TextShader.glsl");

	for (const auto& entry : std::filesystem::directory_iterator(FONTS_DIR))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".ttf")
			m_fontNames.push_back(entry.path().stem().string());
	}
	std::sort(m_fontNames.begin(), m_fontNames.end());

	logInfo("Found {} built-in fonts", m_fontNames.size());

	return !m_shader.isEmpty();
}

const Font* TextSystem::getFont(const std::string& fontName)
{
	std::string name = fontName.empty() ? DEFAULT_FONT_NAME : fontName;
	if (std::find(m_fontNames.begin(), m_fontNames.end(), name) == m_fontNames.end())
	{
		if (m_warnedFontNames.insert(name).second)
			logWarning("Unknown font {}, using {}", name, DEFAULT_FONT_NAME);
		name = DEFAULT_FONT_NAME;
	}

	auto it = m_fonts.find(name);
	if (it != m_fonts.end())
		return it->second.get();

	// A failed font stays cached as null so the error is logged once, not every frame
	std::unique_ptr<Font>& font = m_fonts[name];

	std::string ttfPath = FONTS_DIR + name + ".ttf";

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

namespace
{
	std::vector<glm::vec4> layoutText(const Font& font, const std::string& text, float size)
	{
		std::vector<glm::vec4> vertices;
		float penX = 0;
		float baseline = font.ascender * size;

		for (char c : text)
		{
			if (c == '\n')
			{
				penX = 0;
				baseline += font.lineHeight * size;
				continue;
			}

			unsigned char index = (unsigned char)c;
			const Glyph& glyph = font.glyphs[index < font.glyphs.size() ? index : '?'];

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

				vertices.insert(vertices.end(), { topLeft, bottomLeft, bottomRight, topLeft, bottomRight, topRight });
			}

			penX += glyph.advance * size;
		}

		return vertices;
	}
}

void TextSystem::drawText(TextComponent& text, const glm::mat4& projection)
{
	const Font* font = getFont(text.getFontName());
	if (!font)
		return;

	const float size = text.getFontSize();

	if (text.isDirty())
		text.setMesh(std::make_shared<TextMesh>(layoutText(*font, text.getText(), size)));

	m_shader->use();
	m_shader->setProjectionMatrix(projection);
	m_shader->setUniformValue("offset", text.position);
	m_shader->setUniformValue("color", text.color);
	m_shader->setUniformValue("screenPxRange", glm::max(font->distanceRange * size / font->emSize, 1.0f));
	m_shader->setTextureInShader(font->atlas, "atlas", 0);

	text.getMesh()->draw();
}
