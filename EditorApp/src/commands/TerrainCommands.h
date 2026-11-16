#pragma once

#include "Command.h"
#include "sge.h"

#include <cstdint>
#include <string>
#include <vector>

// A rectangle of texture pixels
struct TextureRect
{
	int x = 0, y = 0, width = 0, height = 0;
};

// The pixels inside one rectangle of a terrain texture (heightmap or layer mask)
struct TextureSnapshot
{
	UUID texture;                 // a resize erases the old heightmap, so a missing texture means the edit is stale
	TextureRect rect;
	int bytesPerPixel = 0;
	std::vector<uint8_t> pixels;  // rect.width * rect.height pixels, row by row, in the texture's own format
};

// Copies the pixels in rect from the texture
TextureSnapshot takeTextureSnapshot(const TextureAssetRef& texture, TextureRect rect);

// The part of a snapshot inside rect (rect must lie within the snapshot's own rect)
TextureSnapshot cropSnapshot(const TextureSnapshot& snapshot, TextureRect rect);

// Puts the snapshot's pixels back; throws if the texture no longer exists
void applyTextureSnapshot(const TextureSnapshot& snapshot);

// The pixels one brush dab touches, clamped to the texture
TextureRect dabRect(glm::vec2 centerPixel, float radiusPixels, const TextureResourceRef& texture);

// Smallest rectangle holding both
TextureRect unionRect(const TextureRect& a, const TextureRect& b);

// A change to part of a terrain texture
class EditTextureCmd : public Command
{
public:
	EditTextureCmd(TextureSnapshot before, TextureSnapshot after, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	TextureSnapshot m_before;
	TextureSnapshot m_after;
	std::string m_label;
};

// Brush dabs on one texture, recorded as one undo step: begin, addDab per dab, end
class TextureStroke
{
public:
	// Keeps a copy of the whole texture; does nothing while a stroke is open
	void begin(const TextureAssetRef& texture);

	bool isOpen() const { return m_open; }
	UUID texture() const { return m_texture.getUID(); }

	// Grows the touched area by one dab
	void addDab(glm::vec2 centerPixel, float radiusPixels);

	// Records the stroke, unless nothing was touched or the texture is gone
	void end(const std::string& label);

private:
	bool m_open = false;
	TextureAssetRef m_texture;
	TextureSnapshot m_start;  // the whole texture from before the stroke
	TextureRect m_rect;       // pixels touched so far
};
