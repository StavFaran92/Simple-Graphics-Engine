#include "core/Configurations.h"

#include "core/Logger.h"

AttributeData getAttributeData(LayoutAttribute attribute)
{
	if (g_attributeMetadata.find(attribute) == g_attributeMetadata.end())
	{
		logError("Unsupported attribute specified.");
		return {};
	}

	return g_attributeMetadata.at(attribute);
}

size_t getAttributeSize(LayoutAttribute attribute)
{
	AttributeData attribData = getAttributeData(attribute);
	return attribData.size;
}

size_t getAttributeCompCount(LayoutAttribute attribute)
{
	AttributeData attribData = getAttributeData(attribute);
	return attribData.length;
}

size_t getAttributeLocationInShader(LayoutAttribute attribute)
{
	AttributeData attribData = getAttributeData(attribute);
	return attribData.location;
}

const std::map<AssetType, std::string> g_assetTypeToStr
{
	{ AssetType::NONE,			"N/A" },
	{ AssetType::TEXTURE,		"textures" },
	{ AssetType::MESH,			"meshes" },
	{ AssetType::ANIMATION,		"animations" },
	{ AssetType::SHADER,		"shaders" },
	{ AssetType::PREFAB,		"prefabs" },
	{ AssetType::MATERIAL,		"materials" },
	{ AssetType::LUA_SCRIPT,	"lua script" },
	{ AssetType::SCENE,			"scene" },
};

const std::map<AssetType, std::string> g_assetTypeToExt
{
	{ AssetType::NONE,			"N/A" },
	{ AssetType::TEXTURE,		".png" },
	{ AssetType::MESH,			".mesh" },
	{ AssetType::ANIMATION,		".anim" },
	{ AssetType::SHADER,		".glsl" },
	{ AssetType::PREFAB,		".prefab" },
	{ AssetType::MATERIAL,		".mat" },
	{ AssetType::LUA_SCRIPT,	".lua" },
	{ AssetType::SCENE,			".scene" },
};