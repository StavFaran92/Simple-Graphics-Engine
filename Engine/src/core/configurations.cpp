#include "Configurations.h"

#include "Logger.h"

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

