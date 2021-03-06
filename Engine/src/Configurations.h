#pragma once

#include <map>
#include <vector>

constexpr bool FLIP_TEXTURE = true;
constexpr bool DEBUG_MODE_ENABLED = false;
constexpr bool DEBUG_DISPLAY_NORMALS = false;

enum class LayoutAttribute
{
	Positions,
	Normals,
	Texcoords,
	Colors
};

const std::map<LayoutAttribute, size_t> g_attributeToSizeMap =
{
	{ LayoutAttribute::Positions, 3},
	{ LayoutAttribute::Normals, 3 },
	{ LayoutAttribute::Texcoords, 2 },
	{ LayoutAttribute::Colors, 3 },
};

size_t getAttributeSize(LayoutAttribute attribute);

const std::map<LayoutAttribute, size_t> g_attributeToLocationMap =
{
	{ LayoutAttribute::Positions, 0},
	{ LayoutAttribute::Normals, 1 },
	{ LayoutAttribute::Texcoords, 2 },
	{ LayoutAttribute::Colors, 3 },
};

size_t getAttributeLocationInShader(LayoutAttribute attribute);

const std::vector<LayoutAttribute> g_defaultLayoutAttributes =
{
	LayoutAttribute::Positions,
	LayoutAttribute::Normals,
	LayoutAttribute::Texcoords,
};