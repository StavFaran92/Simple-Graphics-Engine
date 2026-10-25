#include "text/FontAtlasGenerator.h"

#include <thread>

#include <msdf-atlas-gen/msdf-atlas-gen.h>

#include "core/Logger.h"

bool generateFontAtlas(const std::string& ttfPath, float emSize, float distanceRange, FontAtlasData& out)
{
	using namespace msdf_atlas;

	msdfgen::FreetypeHandle* freetype = msdfgen::initializeFreetype();
	if (!freetype)
	{
		logError("FreeType init failed");
		return false;
	}

	msdfgen::FontHandle* font = msdfgen::loadFont(freetype, ttfPath.c_str());
	if (!font)
	{
		logError("Failed to load font {}", ttfPath);
		msdfgen::deinitializeFreetype(freetype);
		return false;
	}

	std::vector<GlyphGeometry> glyphs;
	FontGeometry geometry(&glyphs);
	geometry.loadCharset(font, 1.0, Charset::ASCII);

	msdfgen::destroyFont(font);
	msdfgen::deinitializeFreetype(freetype);

	for (GlyphGeometry& glyph : glyphs)
		glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, 3.0, 0);

	TightAtlasPacker packer;
	packer.setDimensionsConstraint(DimensionsConstraint::POWER_OF_TWO_SQUARE);
	packer.setScale(emSize);
	packer.setPixelRange(distanceRange);
	packer.setMiterLimit(1.0);
	if (packer.pack(glyphs.data(), (int)glyphs.size()) != 0)
	{
		logError("Failed to pack font atlas for {}", ttfPath);
		return false;
	}

	int width = 0, height = 0;
	packer.getDimensions(width, height);

	int threads = (int)std::thread::hardware_concurrency();
	ImmediateAtlasGenerator<float, 4, mtsdfGenerator, BitmapAtlasStorage<msdfgen::byte, 4>> generator(width, height);
	generator.setThreadCount(threads > 0 ? threads : 1);
	generator.generate(glyphs.data(), (int)glyphs.size());
	msdfgen::BitmapConstRef<msdfgen::byte, 4> bitmap = generator.atlasStorage();

	const msdfgen::FontMetrics& metrics = geometry.getMetrics();

	out.width = width;
	out.height = height;
	out.pixels.assign(bitmap.pixels, bitmap.pixels + 4 * width * height);
	out.emSize = emSize;
	out.distanceRange = distanceRange;
	out.lineHeight = (float)metrics.lineHeight;
	out.ascender = (float)metrics.ascenderY;

	out.glyphs.clear();
	out.glyphs.reserve(glyphs.size());
	for (const GlyphGeometry& glyph : glyphs)
	{
		FontAtlasGlyph g;
		g.codepoint = glyph.getCodepoint();
		g.advance = (float)glyph.getAdvance();
		g.whitespace = glyph.isWhitespace();

		double l, b, r, t;
		glyph.getQuadPlaneBounds(l, b, r, t);
		g.plane = glm::vec4(l, b, r, t);
		glyph.getQuadAtlasBounds(l, b, r, t);
		g.atlas = glm::vec4(l, b, r, t);

		out.glyphs.push_back(g);
	}

	return true;
}
