#pragma once

#include "sge.h"
#include "imgui.h"

#include <string>

// Asset card layout (thumbnail on top, type strip, name + type label below)
class AssetCard
{
public:
	static constexpr float Width = 96.0f;
	static constexpr float Spacing = 8.0f;

	static ImVec2 getSize();
	static ImU32 getTypeColor(AssetType aType);

	// For now every asset uses its type icon; real thumbnails will plug in here
	static ImTextureID getThumbnail(const AssetRecord& aInfo);

	// Submits an invisible button the size of a card at the cursor and draws the card over it.
	// The button is the last item, so IsItemHovered()/IsItemClicked()/GetItemRect*() refer to the whole card.
	static void draw(ImTextureID thumbnail, const std::string& name, const std::string& typeLabel, ImU32 stripColor, bool selected);
	static void draw(const AssetRecord& aInfo, bool selected);
};

// Lays cards out left to right, wrapping to the available width.
// Call begin() once, next() before each card, and end() after the last one.
class AssetCardGrid
{
public:
	void begin();
	void next();
	void end();

private:
	int m_columns = 1;
	int m_count = 0;
};
