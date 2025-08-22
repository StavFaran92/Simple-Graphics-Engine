#pragma once

#include <map>
#include <vector>
#include <string>
#include <typeinfo>

constexpr bool FLIP_TEXTURE = true;
constexpr bool DEBUG_MODE_ENABLED = false;
constexpr bool DEBUG_DISPLAY_NORMALS = false;
static bool SGE_EXPORT_PACKAGE = false;

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

enum class LayoutAttribute// : int
{
	Positions,
	Normals,
	Texcoords,
	Colors,
	Tangents,
	BoneIDs,
	BoneWeights,

	InstanceModel_0,
	InstanceModel_1,
	InstanceModel_2,
	InstanceModel_3,

	InstancePos,
};

struct AttributeData
{
	size_t length = 0;
	size_t size = 0;
	int location = 0;
	std::string typeName;
};

const std::map<LayoutAttribute, AttributeData> g_attributeMetadata =
{
	{ LayoutAttribute::Positions, AttributeData{3, sizeof(float), 0, typeid(float).name()}},
	{ LayoutAttribute::Normals, AttributeData{3, sizeof(float), 1, typeid(float).name()}},
	{ LayoutAttribute::Texcoords, AttributeData{2, sizeof(float), 2, typeid(float).name()}},
	{ LayoutAttribute::Colors, AttributeData{3, sizeof(float), 3, typeid(float).name()}},
	{ LayoutAttribute::Tangents, AttributeData{3, sizeof(float), 4, typeid(float).name()}},
	{ LayoutAttribute::BoneIDs, AttributeData{3, sizeof(int), 5, typeid(int).name()}},
	{ LayoutAttribute::BoneWeights, AttributeData{3, sizeof(float), 6, typeid(float).name()}},

	{ LayoutAttribute::InstanceModel_0, AttributeData{4, sizeof(float), 7, typeid(float).name()}},
	{ LayoutAttribute::InstanceModel_1, AttributeData{4, sizeof(float), 8, typeid(float).name()}},
	{ LayoutAttribute::InstanceModel_2, AttributeData{4, sizeof(float), 9, typeid(float).name()}},
	{ LayoutAttribute::InstanceModel_3, AttributeData{4, sizeof(float), 10, typeid(float).name()}},

	{ LayoutAttribute::InstancePos, AttributeData{3, sizeof(float), 4, typeid(float).name()}},
};

AttributeData getAttributeData(LayoutAttribute attribute);

size_t getAttributeSize(LayoutAttribute attribute);

size_t getAttributeCompCount(LayoutAttribute attribute);

size_t getAttributeLocationInShader(LayoutAttribute attribute);

const std::vector<LayoutAttribute> g_defaultLayoutAttributes =
{
	LayoutAttribute::Positions,
	LayoutAttribute::Normals,
	LayoutAttribute::Texcoords,
};

enum class RigidbodyType : int
{
	Static = 0,
	Dynamic,
	Kinematic
};

struct Color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

enum AssetType : int
{
	NONE = 0,
	TEXTURE,
	MESH,
	ANIMATION,
	SHADER,

	TOTAL_ASSET_TYPE_COUNT
};

static const std::map<AssetType, std::string> g_assetTypeToStr
{
	{ AssetType::NONE,			"N/A"			},
	{ AssetType::TEXTURE,		"textures"		},
	{ AssetType::MESH,			"meshes"		},
	{ AssetType::ANIMATION,		"animations"	},
	{ AssetType::SHADER,		"shaders"		}
};