#pragma once

#include "Command.h"
#include "sge.h"

#include <string>
#include <vector>

// A rectangle of heightmap pixels
struct HeightmapRect
{
	int x = 0, y = 0, width = 0, height = 0;
};

// The heights inside one rectangle of a terrain's heightmap
struct HeightmapSnapshot
{
	entity_id entity = 0;
	UUID heightmap;              // a resize makes a new heightmap, so a different uuid means the edit is stale
	HeightmapRect rect;
	std::vector<float> heights;  // rect.width * rect.height, row by row
};

// Copies the heights in rect from the terrain's heightmap
HeightmapSnapshot takeHeightmapSnapshot(Entity e, HeightmapRect rect);

// The part of a snapshot inside rect (rect must lie within the snapshot's own rect)
HeightmapSnapshot cropSnapshot(const HeightmapSnapshot& snapshot, HeightmapRect rect);

// Puts the snapshot's heights back; throws if the terrain is gone or its heightmap was replaced
void applyHeightmapSnapshot(const HeightmapSnapshot& snapshot);

// The pixels one brush dab touches, clamped to the texture
HeightmapRect dabRect(glm::vec2 centerPixel, float radiusPixels, const TextureResourceRef& texture);

// Smallest rectangle holding both
HeightmapRect unionRect(const HeightmapRect& a, const HeightmapRect& b);

// A change to part of a terrain's heightmap
class EditHeightmapCmd : public Command
{
public:
	EditHeightmapCmd(HeightmapSnapshot before, HeightmapSnapshot after, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	HeightmapSnapshot m_before;
	HeightmapSnapshot m_after;
	std::string m_label;
};
